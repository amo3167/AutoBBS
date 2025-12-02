rm *.jar
javac -cp ../../vendor/oanda_fxtrade.jar ./asirikuy/*.java 
jar cf oanda_helper.jar ./asirikuy
javac -cp ../../vendor/oanda_fxtrade.jar:./oanda_helper.jar *.java 
