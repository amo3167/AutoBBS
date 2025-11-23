
package asirikuy;

import java.io.File;
import java.util.HashSet;
import java.util.Set;
import java.util.Scanner;

import java.awt.image.BufferedImage;
import javax.imageio.ImageIO;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.dukascopy.api.Instrument;
import com.dukascopy.api.system.ClientFactory;
import com.dukascopy.api.system.IClient;
import com.dukascopy.api.system.ISystemListener;

public class DukascopyWrapper {
    private static final Logger LOGGER = LoggerFactory.getLogger(DukascopyWrapper.class);

    public String jnlpUrl;
    public String userName;
    public String password;
    public DumbStrategy dumbStrategy = null;
    private final IClient client;
    private final boolean isDemo;
    private final boolean hasIP;
    private final String tmpDir = System.getProperty("java.io.tmpdir") + "JForex";

    public DukascopyWrapper(String userName, String password, String url, boolean hasStaticIP) throws Exception {
    	this.userName = userName;
        this.password = password;
        this.jnlpUrl = url;
        this.isDemo = url.indexOf("demo") != -1 ? true : false;
        this.hasIP = hasStaticIP;

        //get the instance of the IClient interface
        client = ClientFactory.getDefaultInstance();
        client.setCacheDirectory(new File(tmpDir));
        //set the listener that will receive system events
        client.setSystemListener(new ISystemListener() {
            private int lightReconnects = 3;

        	@Override
        	public void onStart(long processId) {
                LOGGER.info("Strategy started: " + processId);
        	}

			@Override
			public void onStop(long processId) {
                LOGGER.info("Strategy stopped: " + processId);
                if (client.getStartedStrategies().size() == 0) {
                    System.exit(0);
                }
			}

			@Override
			public void onConnect() {
                LOGGER.info("Connected");
                lightReconnects = 100000;
			}

			@Override
			public void onDisconnect() {
                LOGGER.warn("Disconnected");
                if (lightReconnects > 0) {
                    client.reconnect();
                    --lightReconnects;
                    LOGGER.warn(String.format("Trying reconnect: %d attempt", 100000 - lightReconnects));
                } else {
                    try {
                        //sleep for 10 seconds before attempting to reconnect
                        Thread.sleep(10000);
                    } catch (InterruptedException e) {
                        //ignore
                    }
                    try {
                        if (!isDemo && !hasIP) {
                            client.connect(jnlpUrl, DukascopyWrapper.this.userName,
                                    DukascopyWrapper.this.password, getCaptcha());
                        } else {
                            client.connect(jnlpUrl, DukascopyWrapper.this.userName,
                                    DukascopyWrapper.this.password);
                        }
                    } catch (Exception e) {
                        LOGGER.error(e.getMessage(), e);
                    }
                }
			}
		});

        LOGGER.info("Connecting...");

        if (!isDemo && !hasIP) {
            client.connect(jnlpUrl, userName, password, getCaptcha());
        } else {
            client.connect(jnlpUrl, userName, password);
        }

        //wait for it to connect
        int i = 10; //wait max ten seconds
        while (i > 0 && !client.isConnected()) {
            Thread.sleep(1000);
            i--;
        }
        if (!client.isConnected()) {
            LOGGER.error("Failed to connect Dukascopy servers");
            System.exit(1);
        }
       
        //workaround for LoadNumberOfCandlesAction for JForex-API versions > 2.6.64
        Thread.sleep(5000);
        
        //subscribe to the instruments
        Set<Instrument> instruments = new HashSet<Instrument>();
        instruments.add(Instrument.EURUSD);
        instruments.add(Instrument.GBPUSD);
        instruments.add(Instrument.USDJPY);
        instruments.add(Instrument.EURJPY);
        instruments.add(Instrument.AUDUSD);
        instruments.add(Instrument.USDCAD);
        instruments.add(Instrument.GBPJPY);
        instruments.add(Instrument.USDCHF);
        instruments.add(Instrument.EURGBP);
        instruments.add(Instrument.AUDJPY);
        
        LOGGER.info("Subscribing instruments...");
        client.setSubscribedInstruments(instruments);
        
        //start the strategy
        LOGGER.info("Starting strategy");
        this.dumbStrategy = new DumbStrategy();
        client.startStrategy(dumbStrategy);
        //now it's running
    }
    
    public void disconnect(){
    	client.stopStrategy(0);
    	client.disconnect();
    }
    
    private String getCaptcha() {
        String res = "";
        try {
            BufferedImage img = client.getCaptchaImage(jnlpUrl);
            String fileName = tmpDir + "/fxdd.gif";
            ImageIO.write(img, "gif", new File(fileName));
            Scanner reader = new Scanner(System.in);
            System.out.print("Enter PIN from \"" + fileName + "\": ");
            res = reader.next();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return res;
    }

}
