package PortfolioResult.PortfolioResult;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

/**
 * Reads configuration properties from the config.properties file.
 * Properties file should be located in the classpath resources.
 */
public class ConfigReader {

	/**
	 * Default constructor.
	 */
	public ConfigReader() {
	}

	/**
	 * Retrieves a property value from the configuration file.
	 * 
	 * @param key the property key to retrieve
	 * @return the property value, or empty string if not found
	 * @throws IOException if there's an error reading the properties file
	 */
	public String getPropValues(String key) throws IOException {
		String result = "";
		InputStream inputStream = null;
		
		try {
			Properties prop = new Properties();
			String propFileName = "config.properties";

			inputStream = getClass().getClassLoader().getResourceAsStream(propFileName);

			if (inputStream != null) {
				prop.load(inputStream);
			} else {
				throw new FileNotFoundException("property file '" + propFileName + 
						"' not found in the classpath");
			}

			result = prop.getProperty(key);
			
		} catch (Exception e) {
			System.out.println("Exception: " + e);
		} finally {
			if (inputStream != null) {
				inputStream.close();
			}
		}
		return result;
	}
}
