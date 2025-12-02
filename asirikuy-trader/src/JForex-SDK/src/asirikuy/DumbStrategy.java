package asirikuy;

import java.math.BigInteger;
import java.security.SecureRandom;
import java.util.ArrayList;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.dukascopy.api.IAccount;
import com.dukascopy.api.IBar;
import com.dukascopy.api.IConsole;
import com.dukascopy.api.IContext;
import com.dukascopy.api.IEngine;
import com.dukascopy.api.IHistory;
import com.dukascopy.api.IMessage;
import com.dukascopy.api.IOrder;
import com.dukascopy.api.IStrategy;
import com.dukascopy.api.ITick;
import com.dukascopy.api.Instrument;
import com.dukascopy.api.JFException;
import com.dukascopy.api.OfferSide;
import com.dukascopy.api.Period;

public class DumbStrategy implements IStrategy {
	public IEngine engine = null;
	private IHistory history;
	private int tagCounter = 0;
	private IConsole console;
	public IAccount account;
	public ITick tick;

	public List<TradeSignal> pendingOrders;
	public List<TradeSignalResponse> resultOrders;
	private static final Logger LOGGER = LoggerFactory
			.getLogger(DukascopyWrapper.class);

	public List<Double> getBidAsk(String stringInstrument) throws JFException {
		List<Double> bidAsk = new ArrayList<Double>();
		Instrument instrument = Instrument.fromString(stringInstrument);
		ITick tick = history.getLastTick(instrument);

		if (tick == null)
			return null;
		bidAsk.add(tick.getBid());
		bidAsk.add(tick.getAsk());
		return bidAsk;
	}

	public List<IBar> getCandles(String stringPeriod, String stringInstrument,
			int numCandles) throws JFException {
		Instrument instrument = Instrument.fromString(stringInstrument);
		Period period;

		if (stringPeriod.equalsIgnoreCase("h1"))
			period = Period.ONE_HOUR;
		else if (stringPeriod.equalsIgnoreCase("d1"))
			period = Period.DAILY;
		else
			return null;

		ITick tick = history.getLastTick(instrument);
		long prevBarTime = history.getPreviousBarStart(period, tick.getTime());
		List<IBar> bars = history
				.getBars(instrument, period, OfferSide.BID, history
						.getTimeForNBarsBack(period, prevBarTime,
								numCandles - 1), prevBarTime);
		bars.add(history.getBar(instrument, period, OfferSide.BID, 0));

		return bars;
	}

	public void closeOrder(String orderID) throws JFException,
	InterruptedException {

		TradeSignal signal = new TradeSignal();
		signal.command = TradeSignal.CommandEnum.CLOSE;
		signal.parameters.put("orderID", orderID);
		synchronized (pendingOrders) {
			pendingOrders.add(signal);
		}
		
	}

	public void modifyOrder(String orderID, double stopLoss, double takeProfit)
			throws InterruptedException {

		TradeSignal signal = new TradeSignal();
		signal.command = TradeSignal.CommandEnum.MODIFY;
		signal.parameters.put("orderID", orderID);
		signal.parameters.put("stopLoss", new Double(stopLoss));
		signal.parameters.put("takeProfit", new Double(takeProfit));
		synchronized (pendingOrders) {
			pendingOrders.add(signal);
		}
	}

	public int checkOrder(String orderPseudoId, String message) {
		int result = -1; // -- -1 es sin definir respuesta
		TradeSignalResponse found = null;
		synchronized (resultOrders) {
			for (TradeSignalResponse response : resultOrders) {
				if (response.identifier.equals(orderPseudoId)) {
					found = response;
					result = (response.result) ? 1 : 0;
					//message.append(response.message);
					message = message + response.message;
					break;
				}
			}

			if (null != found)
				resultOrders.remove(found);
		}
		return result;
	}

	public void openOrder(String magicLabel, double lots, String direction,
			String pair, double stopLoss, double takeProfit, double slippage,
			String comment) throws InterruptedException {

		TradeSignal signal = new TradeSignal();
		signal.command = TradeSignal.CommandEnum.OPEN;
		signal.parameters.put("magicLabel", magicLabel);
		signal.parameters.put("lots", new Double(lots));
		signal.parameters.put("direction", direction);
		signal.parameters.put("pair", pair);
		signal.parameters.put("stopLoss", new Double(stopLoss));
		signal.parameters.put("takeProfit", new Double(takeProfit));
		signal.parameters.put("slippage", new Double(slippage));
		signal.parameters.put("comment", comment);

		synchronized (pendingOrders) {
			pendingOrders.add(signal);
		}
	}

	public void onStart(IContext context) throws JFException {
		engine = context.getEngine();
		this.console = context.getConsole();
		history = context.getHistory();
		pendingOrders = Collections
				.synchronizedList(new LinkedList<TradeSignal>());
		resultOrders = Collections
				.synchronizedList(new LinkedList<TradeSignalResponse>());
	}

	public void onStop() throws JFException {
		for (IOrder order : engine.getOrders()) {
			order.close();
		}
		console.getOut().println("Stopped");
	}

	public void onTick(Instrument instrument, ITick tick) throws JFException {
		this.tick = tick;
		processPendingOrders();
	}

	public void onBar(Instrument instrument, Period period, IBar askBar,
			IBar bidBar) {
	}

	// count open positions
	protected int positionsTotal(Instrument instrument) throws JFException {
		int counter = 0;
		for (IOrder order : engine.getOrders(instrument)) {
			if (order.getState() == IOrder.State.FILLED) {
				counter++;
			}
		}
		return counter;
	}

	public static double roundToDecimals(double d, int c) {
		int temp = (int) ((d * Math.pow(10, c)));
		return (((double) temp) / Math.pow(10, c));
	}

	private void processPendingOrders() throws JFException {
		int numOrders = 0;
		String label = "";
		List<TradeSignal> signals = new ArrayList<TradeSignal>();
		synchronized (pendingOrders) {
			for (TradeSignal signal : pendingOrders) {
				IOrder order;
				boolean result = false;
				final SecureRandom random = new SecureRandom();
				String message = null;
				switch (signal.command) {
				case OPEN:
					Instrument instrument = Instrument
					.fromString((String) signal.parameters.get("pair"));
					IEngine.OrderCommand orderCommand;
					int pipDecimals = instrument.getPipScale();
					double stopLoss = roundToDecimals(
							(Double) signal.parameters.get("stopLoss"),
							pipDecimals);
					double takeProfit = roundToDecimals(
							(Double) signal.parameters.get("takeProfit"),
							pipDecimals);
					if ("sell".equalsIgnoreCase((String) signal.parameters
							.get("direction")))
						orderCommand = IEngine.OrderCommand.SELL;
					else
						orderCommand = IEngine.OrderCommand.BUY;

					label = (String) signal.parameters.get("magicLabel");
					if (label.equalsIgnoreCase("")) {
						java.util.Date date = new java.util.Date();
						Double epoch = (double) date.getTime();
						label = String.format("ID%d", epoch.intValue());
						label = String.format("ID%s", new BigInteger(130,
								random).toString(32));
					}
					LOGGER.info(String.format(
							"Opening order: %s %s %.5f %.5f %.5f %s",
							(String) signal.parameters.get("direction"), label,
							(Double) signal.parameters.get("lots"), stopLoss,
							takeProfit,
							(String) signal.parameters.get("comment")));
					try {
						order = engine.submitOrder(label, instrument,
								orderCommand,
								(Double) signal.parameters.get("lots"), 0,
								(Double) signal.parameters.get("slippage"),
								stopLoss, takeProfit, 0,
								(String) signal.parameters.get("comment"));
						if (order.getState().equals(IOrder.State.CREATED)) {
							result = true;
						}
					} catch (Exception e) {
						result = false;
						message = String.format("Error opening order: %s",
								e.getMessage());
						LOGGER.error(message);
					}
					TradeSignalResponse response = new TradeSignalResponse();
					response.identifier = label;
					response.signal = signal;
					response.result = result;
					response.message = message;
					synchronized (resultOrders) {
						resultOrders.add(response);
					}

					break;

				case CLOSE:
					order = engine.getOrderById((String) signal.parameters
							.get("orderID"));
					LOGGER.info(String.format("Closing order: %s",
							(String) signal.parameters.get("orderID")));
					if (order != null) {
						try {
							order.close();
							result = true;
						} catch (Exception e) {
							result = false;
							message = String.format("Error closing order: %s",
									e.getMessage());
							LOGGER.error(message);
						}
					} else {
						result = false;
						message = String.format("Order: %s doesn't exist",
								(String) signal.parameters.get("orderID"));
						LOGGER.error(message);
					}
					response = new TradeSignalResponse();
					response.identifier = (String) signal.parameters
							.get("orderID");
					response.signal = signal;
					response.result = result;
					response.message = message;
					synchronized (resultOrders) {
						resultOrders.add(response);
					}
					break;

				case MODIFY:
					order = engine.getOrderById((String) signal.parameters
							.get("orderID"));
					pipDecimals = order.getInstrument().getPipScale();
					stopLoss = roundToDecimals(
							(Double) signal.parameters.get("stopLoss"),
							pipDecimals);
					takeProfit = roundToDecimals(
							(Double) signal.parameters.get("takeProfit"),
							pipDecimals);
					LOGGER.info(String.format(
							"Modifyin order: %s SL: %.5f TP: %.5f",
							(String) signal.parameters.get("orderID"),
							stopLoss, takeProfit));
					if (order != null) {
						try {
							order.setStopLossPrice(stopLoss);
							order.setTakeProfitPrice(takeProfit);
							result = true;
						} catch (Exception e) {
							result = false;
							message = String.format("Error modifying order: %s", e.getMessage());
							LOGGER.error(message);
						}
					} else {
						result = false;
						message = String.format("Order: %s doesn't exist",
								(String) signal.parameters.get("orderID"));
						LOGGER.error(message);
					}
					response = new TradeSignalResponse();
					response.identifier = (String) signal.parameters
							.get("orderID");
					response.signal = signal;
					response.result = result;
					response.message = message;
					synchronized (resultOrders) {
						resultOrders.add(response);
					}
					break;
				}
			}
			pendingOrders.clear();
		}
	}

	protected String getLabel(Instrument instrument) {
		String label = instrument.name();
		label = label.substring(0, 2) + label.substring(3, 5);
		label = label + (tagCounter++);
		label = label.toLowerCase();
		return label;
	}

	public void onMessage(IMessage message) throws JFException {
	}

	public void onAccount(IAccount account) throws JFException {
		this.account = account;
	}

}
