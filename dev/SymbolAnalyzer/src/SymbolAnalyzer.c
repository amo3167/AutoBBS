/**
 * @file
 * @brief     Parses and provides detailed information about forex currencies and symbols (instruments).
 * @details   Symbols are broken down into prefix, base currency, separator, quote currency, and suffix.
 * @details   This allows for many non-standardized symbol names to be identified reliably.
 * @details   Detailed information can be obtained for a currency including the currency name, number, 
 * @details   the number of digits after the decimal separator, and the locations where the currency is used.
 * 
 * @author    Morgan Doel (Initial implementation)
 * @author    Daniel Fernandez (Assisted with design and code styling)
 * @author    Maxim Feinshtein (Assisted with design and code styling)
 * @version   F4.x.x
 * @date      2012
 *
 * @copyright END-USER LICENSE AGREEMENT FOR ASIRIKUY SOFTWARE. IMPORTANT PLEASE READ THE TERMS AND CONDITIONS OF THIS LICENSE AGREEMENT CAREFULLY BEFORE USING THIS SOFTWARE: 
 * @copyright Asirikuy's End-User License Agreement ("EULA") is a legal agreement between you (either an individual or a single entity) and Asirikuy for the use of the Asirikuy Framework in both source and binary forms. By installing, copying, or otherwise using the Asirikuy Framework, you agree to be bound by the terms of this EULA. This license agreement represents the entire agreement concerning the program between you and Asirikuy, (referred to as "licenser"), and it supersedes any prior proposal, representation, or understanding between the parties. If you do not agree to the terms of this EULA, do not install or use the Asirikuy Framework.
 * @copyright The Asirikuy Framework is protected by copyright laws and international copyright treaties, as well as other intellectual property laws and treaties. The Asirikuy Framework is licensed, not sold.
 * @copyright 1. GRANT OF LICENSE.
 * @copyright The Asirikuy Framework is licensed as follows:
 * @copyright (a) Installation and Use.
 * @copyright Asirikuy grants you the right to install and use copies of the Asirikuy Framework in both source and binary forms for personal and business use. You may also make modifications to the source code.
 * @copyright (b) Backup Copies.
 * @copyright You may make copies of the Asirikuy Framework as may be necessary for backup and archival purposes.
 * @copyright 2. DESCRIPTION OF OTHER RIGHTS AND LIMITATIONS.
 * @copyright (a) Maintenance of Copyright Notices.
 * @copyright You must not remove or alter any copyright notices on any and all copies of the Asirikuy Framework.
 * @copyright (b) Distribution.
 * @copyright You may not distribute copies of the Asirikuy Framework in binary or source forms to third parties outside of the Asirikuy community.
 * @copyright (c) Rental.
 * @copyright You may not rent, lease, or lend the Asirikuy Framework.
 * @copyright (d) Compliance with Applicable Laws.
 * @copyright You must comply with all applicable laws regarding use of the Asirikuy Framework.
 * @copyright 3. TERMINATION
 * @copyright Without prejudice to any other rights, Asirikuy may terminate this EULA if you fail to comply with the terms and conditions of this EULA. In such event, you must destroy all copies of the Asirikuy Framework in your possession.
 * @copyright 4. COPYRIGHT
 * @copyright All title, including but not limited to copyrights, in and to the Asirikuy Framework and any copies thereof are owned by Asirikuy or its suppliers. All title and intellectual property rights in and to the content which may be accessed through use of the Asirikuy Framework is the property of the respective content owner and may be protected by applicable copyright or other intellectual property laws and treaties. This EULA grants you no rights to use such content. All rights not expressly granted are reserved by Asirikuy.
 * @copyright 5. NO WARRANTIES
 * @copyright Asirikuy expressly disclaims any warranty for the Asirikuy Framework. The Asirikuy Framework is provided 'As Is' without any express or implied warranty of any kind, including but not limited to any warranties of merchantability, noninfringement, or fitness of a particular purpose. Asirikuy does not warrant or assume responsibility for the accuracy or completeness of any information, text, graphics, links or other items contained within the Asirikuy Framework. Asirikuy makes no warranties respecting any harm that may be caused by the transmission of a computer virus, worm, time bomb, logic bomb, or other such computer program. Asirikuy further expressly disclaims any warranty or representation to Authorized Users or to any third party.
 * @copyright 6. LIMITATION OF LIABILITY
 * @copyright In no event shall Asirikuy or any contributors to the Asirikuy Framework be liable for any damages (including, without limitation, lost profits, business interruption, or lost information) rising out of 'Authorized Users' use of or inability to use the Asirikuy Framework, even if Asirikuy has been advised of the possibility of such damages. In no event will Asirikuy or any contributors to the Asirikuy Framework be liable for loss of data or for indirect, special, incidental, consequential (including lost profit), or other damages based in contract, tort or otherwise. Asirikuy and contributors to the Asirikuy Framework shall have no liability with respect to the content of the Asirikuy Framework or any part thereof, including but not limited to errors or omissions contained therein, libel, infringements of rights of publicity, privacy, trademark rights, business interruption, personal injury, loss of privacy, moral rights or the disclosure of confidential information.
 */

#include "Precompiled.h"
#include "SymbolAnalyzer.h"

#define TOTAL_CURRENCY_INFO_INDEXES 5
#define TOTAL_CURRENCY_TYPE_INDEXES 2
#define TOTAL_CURRENCIES            187
#define TOTAL_CURRENCY_PAIRS        125
#define CURRENCY_INFO_STRING_SIZE   250
#define CURRENCY_SIZE               10
#define PRE_SEP_SUF_SIZE            10

typedef enum currencyInfoIndexes_t
{
  CURRENCY_CODE        = 0,
  CURRENCY_NUMBER      = 1,
  DIGITS_AFTER_DECIMAL = 2,
  CURRENCY_NAME        = 3,
  CURRENCY_LOCATIONS   = 4
} CurrencyInfoIndexes;

typedef enum currencyTypeIndexes_t
{
  BASE_CURRENCY  = 0,
  QUOTE_CURRENCY = 1
} CurrencyTypeIndexes;

static const char g_currencyPairs[TOTAL_CURRENCY_PAIRS][TOTAL_CURRENCY_TYPE_INDEXES][CURRENCY_SIZE] =
{
	/* References: instaforex, alpari UK, alpari NZ
	   Base Currency, Quote Currency */

	"EUR", "USD",
	"GBP", "USD",
	"USD", "JPY",
	"USD", "CHF",
	"USD", "CAD",
	"AUD", "USD",
	"NZD", "USD",
	"EUR", "JPY",
	"EUR", "CHF",
	"EUR", "GBP",
	"AUD", "CAD",
	"AUD", "CHF",
	"AUD", "JPY",
	"CAD", "CHF",
	"CAD", "JPY",
	"CHF", "JPY",
	"NZD", "CAD",
	"NZD", "CHF",
	"NZD", "JPY",
	"EUR", "AUD",
	"GBP", "CHF",
	"GBP", "JPY",
	"AUD", "NZD",
	"EUR", "CAD",
	"EUR", "NZD",
	"GBP", "AUD",
	"GBP", "CAD",
	"GBP", "NZD",
	"USD", "DKK",
	"USD", "NOK",
	"USD", "SEK",
	"USD", "ZAR",
	"AUD", "CZK",
	"AUD", "DKK",
	"AUD", "HKD",
	"AUD", "HUF",
	"AUD", "MXN",
	"AUD", "NOK",
	"AUD", "PLN",
	"AUD", "SEK",
	"AUD", "SGD",
	"AUD", "ZAR",
	"CAD", "CZK",
	"CAD", "DKK",
	"CAD", "HKD",
	"CAD", "HUF",
	"CAD", "MXN",
	"CAD", "NOK",
	"CAD", "PLN",
	"CAD", "SEK",
	"CAD", "SGD",
	"CAD", "ZAR",
	"CHF", "CZK",
	"CHF", "DKK",
	"CHF", "HKD",
	"CHF", "HUF",
	"CHF", "MXN",
	"CHF", "NOK",
	"CHF", "PLN",
	"CHF", "SEK",
	"CHF", "SGD",
	"CHF", "ZAR",
	"EUR", "CZK",
	"EUR", "DKK",
	"EUR", "HKD",
	"EUR", "HUF",
	"EUR", "MXN",
	"EUR", "NOK",
	"EUR", "PLN",
	"EUR", "SEK",
	"EUR", "SGD",
	"EUR", "ZAR",
	"GBP", "CZK",
	"GBP", "DKK",
	"GBP", "HKD",
	"GBP", "HUF",
	"GBP", "MXN",
	"GBP", "NOK",
	"GBP", "PLN",
	"GBP", "SEK",
	"GBP", "SGD",
	"GBP", "ZAR",
	"NZD", "CZK",
	"NZD", "DKK",
	"NZD", "HKD",
	"NZD", "HUF",
	"NZD", "MXN",
	"NZD", "NOK",
	"NZD", "PLN",
	"NZD", "SEK",
	"NZD", "SGD",
	"NZD", "ZAR",
	"USD", "CZK",
	"USD", "HKD",
	"USD", "HUF",
	"USD", "MXN",
	"USD", "SGD",
	"USD", "PLN",
	"CZK", "JPY",
	"DKK", "JPY",
	"HKD", "JPY",
	"HUF", "JPY",
	"MXN", "JPY",
	"NOK", "JPY",
	"SGD", "JPY",
	"SEK", "JPY",
	"ZAR", "JPY",
	"NZD", "HKD",
	"USD", "TRY",
	"EUR", "TRY",
	"USD", "RUR",
	"XAG", "EUR",
	"XAG", "USD",
	"XAU", "AUD",
	"XAU", "EUR",
	"XAU", "USD",
	"XTI", "USD",
	"BTC", "USD",
	"ETH", "USD",
	"US500", "USD",
	"AUS200", "AUD",
	"GER30", "EUR",
	"USTEC", "USD",
	"NAS100", "USD",
	"SpotCrude","USD"
};

static const char g_currencies[TOTAL_CURRENCIES][TOTAL_CURRENCY_INFO_INDEXES][CURRENCY_INFO_STRING_SIZE] =
{
	/* Reference: ISO 4217
	   Currency code, Number, Number of digits after the decimal separator, Name, Locations */

	"AED", "784", "2", "United Arab Emirates dirham", "United Arab Emirates",
	"AFN", "971", "2", "Afghan afghani", "Afghanistan",
	"ALL", "008", "2", "Albanian lek", "Albania",
	"AMD", "051", "2", "Armenian dram", "Armenia",
	"ANG", "532", "2", "Netherlands Antillean guilder/florin", "	Cura�ao, Sint Maarten",
	"AOA", "973", "2", "Angolan kwanza", "Angola",
	"ARS", "032", "2", "Argentine peso", "Argentina",
	"AUD", "036", "2", "Australian dollar", "Australia, Australian Antarctic Territory, Christmas Island, Cocos (Keeling) Islands, Heard and McDonald Islands, Kiribati, Nauru, Norfolk Island, Tuvalu",
	"AWG", "533", "2", "Aruban guilder", "Aruba",
	"AZN", "944", "2", "Azerbaijanian manat", "Azerbaijan",
	"BAM", "977", "2", "Convertible marks", "Bosnia and Herzegovina",
	"BBD", "052", "2", "Barbados dollar", "Barbados",
	"BDT", "050", "2", "Bangladeshi taka", "Bangladesh",
	"BGN", "975", "2", "Bulgarian lev", "Bulgaria",
	"BHD", "048", "3", "Bahraini dinar", "Bahrain",
	"BIF", "108", "0", "Burundian franc", "Burundi",
	"BMD", "060", "2", "Bermuda dollar", "Bermuda",
	"BND", "096", "2", "Brunei dollar", "Brunei, Singapore",
	"BOB", "068", "2", "Boliviano", "Bolivia",
	"BOV", "984", "2", "Bolivian Mvdol (funds code)", "Bolivia",
	"BRL", "986", "2", "Brazilian real", "Brazil",
	"BSD", "044", "2", "Bahamian dollar", "Bahamas",
	"BTN", "064", "2", "Bhutanese Ngultrum", "Bhutan",
	"BWP", "072", "2", "Botswana Pula", "Botswana",
	"BYR", "974", "0", "Belarussian ruble", "Belarus",
	"BZD", "084", "2", "Belize dollar", "Belize",
	"CAD", "124", "2", "Canadian dollar", "Canada",
	"CDF", "976", "2", "Franc Congolais", "Democratic Republic of Congo",
	"CHE", "947", "2", "WIR euro (complementary currency)", "Switzerland",
	"CHF", "756", "2", "Swiss franc", "Switzerland , Liechtenstein",
	"CHW", "948", "2", "WIR franc (complementary currency)", "Switzerland",
	"CLF", "990", "0", "Unidad de Fomento (funds code)", "Chile",
	"CLP", "152", "0", "Chilean peso", "Chile",
	"CNY", "156", "2", "Chinese yuan", "Mainland China",
	"COP", "170", "2", "Colombian peso", "Colombia",
	"COU", "970", "2", "Unidad de Valor Real", "Colombia",
	"CRC", "188", "2", "Costa Rican colon", "Costa Rica",
	"CUP", "192", "2", "Cuban peso", "Cuba",
	"CVE", "132", "0", "Cape Verde escudo", "Cape Verde",
	"CZK", "203", "2", "Czech koruna", "Czech Republic",
	"DJF", "262", "0", "Djibouti franc", "Djibouti",
	"DKK", "208", "2", "Danish krone", "Denmark, Faroe Islands, Greenland",
	"DOP", "214", "2", "Dominican peso", "Dominican Republic",
	"DZD", "012", "2", "Algerian dinar", "Algeria",
	"EEK", "233", "2", "Estonian Kroon", "Estonia",
	"EGP", "818", "2", "Egyptian pound", "Egypt",
	"ERN", "232", "2", "Eritrean Nakfa","Eritrea",
	"ETB", "230", "2", "Ethiopian birr","Ethiopia",
	"EUR", "978", "2", "Euro", "Some European Union countries",
	"FJD", "242", "2", "Fiji dollar", "Fiji",
	"FKP", "238", "2", "Falkland Islands pound", "Falkland Islands",
	"GBP", "826", "2", "Pound sterling", "United Kingdom, Crown Dependencies (the Isle of Man and the Channel Islands), certain British Overseas Territories (South Georgia and the South Sandwich Islands, British Antarctic Territory and British Indian, Ocean Territory)",
	"GEL", "981", "2", "Georgian lari", "Georgia",
	"GHS", "936", "2", "Ghanaian cedi", "Ghana",
	"GIP", "292", "2", "Gibraltar pound", "Gibraltar",
	"GMD", "270", "2", "Gambian dalasi", "Gambia",
	"GNF", "324", "0", "Guinea franc", "Guinea",
	"GTQ", "320", "2", "Guatemalan quetzal", "Guatemala",
	"GYD", "328", "2", "Guyana dollar", "Guyana",
	"HKD", "344", "2", "Hong Kong dollar", "Hong Kong Special Administrative Region",
	"HNL", "340", "2", "Honduran lempira", "Honduras",
	"HRK", "191", "2", "Croatian kuna", "Croatia",
	"HTG", "332", "2", "Haiti gourde", "Haiti",
	"HUF", "348", "2", "Hungarian forint", "Hungary",
	"IDR", "360", "0", "Indonesian rupiah", "Indonesia",
	"ILS", "376", "2", "Israeli new sheqel", "Israel",
	"INR", "356", "2", "Indian rupee", "Bhutan, India",
	"IQD", "368", "0", "Iraqi dinar", "Iraq",
	"IRR", "364", "0", "Iranian rial", "Iran",
	"ISK", "352", "0", "Iceland krona", "Iceland",
	"JMD", "388", "2", "Jamaican dollar", "Jamaica",
	"JOD", "400", "3", "Jordanian dinar", "Jordan",
	"JPY", "392", "0", "Japanese yen", "Japan",
	"KES", "404", "2", "Kenyan shilling", "Kenya",
	"KGS", "417", "2", "Kyrgyzstani som", "Kyrgyzstan",
	"KHR", "116", "2", "Cambodian riel", "Cambodia",
	"KMF", "174", "0", "Comoro franc", "Comoros",
	"KPW", "408", "0", "North Korean won", "North Korea",
	"KRW", "410", "0", "South Korean won", "South Korea",
	"KWD", "414", "3", "Kuwaiti dinar", "Kuwait",
	"KYD", "136", "2", "Cayman Islands dollar", "Cayman Islands",
	"KZT", "398", "2", "Kazakhstani Tenge", "Kazakhstan",
	"LAK", "418", "0", "Lao Kip", "Laos",
	"LBP", "422", "0", "Lebanese pound", "Lebanon",
	"LKR", "144", "2", "Sri Lanka rupee", "Sri Lanka",
	"LRD", "430", "2", "Liberian dollar", "Liberia",
	"LSL", "426", "2", "Lesotho Loti", "Lesotho",
	"LTL", "440", "2", "Lithuanian litas", "Lithuania",
	"LVL", "428", "2", "Latvian lats", "Latvia",
	"LYD", "434", "3", "Libyan dinar", "Libya",
	"MAD", "504", "2", "Moroccan dirham", "Morocco, Western Sahara",
	"MDL", "498", "2", "Moldovan leu", "Moldova",
	"MGA", "969", "0", "Malagasy ariary", "Madagascar",
	"MKD", "807", "2", "Macedonian Denar", "Former Yugoslav, Republic of Macedonia",
	"MMK", "104", "0", "Myanma Kyat", "Myanmar",
	"MNT", "496", "2", "Mongolian Tugrik", "Mongolia",
	"MOP", "446", "1", "Macanese Pataca", "Macau Special Administrative Region",
	"MRO", "478", "0", "Mauritanian ouguiya", "Mauritania",
	"MUR", "480", "2", "Mauritius rupee", "Mauritius",
	"MVR", "462", "2", "Maldivian rufiyaa", "Maldives",
	"MWK", "454", "2", "Malawian kwacha", "Malawi",
	"MXN", "484", "2", "Mexican peso", "Mexico",
	"MXV", "979", "2", "Mexican Unidad de Inversion (UDI) (funds code)", "Mexico",
	"MYR", "458", "2", "Malaysian ringgit", "Malaysia",
	"MZN", "943", "2", "Mozambican metical", "Mozambique",
	"NAD", "516", "2", "Namibian dollar", "Namibia",
	"NGN", "566", "2", "Nigerian naira", "Nigeria",
	"NIO", "558", "2", "Cordoba oro", "Nicaragua",
	"NOK", "578", "2", "Norwegian krone", "Norway",
	"NPR", "524", "2", "Nepalese rupee", "Nepal",
	"NZD", "554", "2", "New Zealand dollar", "Cook Islands, New Zealand, Niue, Pitcairn, Tokelau",
	"OMR", "512", "3", "Omani Rial", "Oman",
	"PAB", "590", "2", "Panamanian balboa", "Panama",
	"PEN", "604", "2", "Peruvian nuevo sol", "Peru",
	"PGK", "598", "2", "Papua New Guinean kina", "Papua New Guinea",
	"PHP", "608", "2", "Philippine peso", "Philippines",
	"PKR", "586", "2", "Pakistan rupee", "Pakistan",
	"PLN", "985", "2", "Polish zloty", "Poland",
	"PYG", "600", "0", "Paraguayan Guarani", "Paraguay",
	"QAR", "634", "2", "Qatari rial", "Qatar",
	"RON", "946", "2", "Romanian new leu", "Romania",
	"RSD", "941", "2", "Serbian dinar", "Serbia",
	"RUB", "643", "2", "Russian ruble", "Russia, Abkhazia, South Ossetia",
	"RWF", "646", "0", "Rwandan franc", "Rwanda",
	"SAR", "682", "2", "Saudi riyal", "Saudi Arabia",
	"SBD", "090", "2", "Solomon Islands dollar", "Solomon Islands",
	"SCR", "690", "2", "Seychelles rupee", "Seychelles",
	"SDG", "938", "2", "Sudanese pound", "Sudan",
	"SEK", "752", "2", "Swedish krona", "Sweden",
	"SGD", "702", "2", "Singapore dollar", "Singapore, Brunei",
	"SHP", "654", "2", "Saint Helena pound", "Saint Helena",
	"SKK", "703", "2", "Slovak koruna", "Slovakia",
	"SLL", "694", "0", "Sierra Leonean Leone", "Sierra Leone",
	"SOS", "706", "2", "Somali shilling", "Somalia",
	"SRD", "968", "2", "Surinam dollar", "Suriname",
	"STD", "678", "0", "S�o Tom?and Pr�ncipe Dobra", "S�o Tom?and Pr�ncipe",
	"SYP", "760", "2", "Syrian pound", "Syria",
	"SZL", "748", "2", "Lilangeni", "Swaziland",
	"THB", "764", "2", "Thai baht", "Thailand",
	"TJS", "972", "2", "Tajikistani somoni", "Tajikistan",
	"TMM", "795", "2", "Turkmenistani manat", "Turkmenistan",
	"TND", "788", "3", "Tunisian dinar", "Tunisia",
	"TOP", "776", "2", "Tongan pa\'anga", "Tonga",
	"TRY", "949", "2", "New Turkish lira", "Turkey",
	"TTD", "780", "2", "Trinidad and Tobago dollar", "Trinidad and Tobago",
	"TWD", "901", "2", "New Taiwan dollar", "Taiwan and other islands that are under the effective control of the Republic of China (ROC)",
	"TZS", "834", "2", "Tanzanian shilling", "Tanzania",
	"UAH", "980", "2", "Ukrainian hryvnia", "Ukraine",
	"UGX", "800", "0", "Uganda shilling", "Uganda",
	"USD", "840", "2", "US dollar", "American Samoa, British Indian Ocean Territory, Ecuador, El Salvador, Guam, Haiti, Marshall Islands, Micronesia, Northern Mariana Islands, Palau, Panama, Puerto Rico, Timor-Leste, Turks and Caicos Islands, United States, Virgin Islands",
	"USN", "997", "2", "United States dollar (next day) (funds code)", "United States",
	"USS", "998", "2", "United States dollar (same day) (funds code) (one source claims it is no longer used, but it is still on the ISO 4217-MA list)", "United States",
	"UYU", "858", "2", "Uruguayan Peso", "Uruguayo, Uruguay",
	"UZS", "860", "2", "Uzbekistan som", "Uzbekistan",
	"VEF", "937", "2", "Venezuelan bol�var fuerte", "Venezuela",
	"VND", "704", "0", "Vietnamese dong", "Vietnam",
	"VUV", "548", "0", "Vanuatu Vatu", "Vanuatu",
	"WST", "882", "2", "Samoan tala", "Samoa",
	"XAF", "950", "0", "CFA franc BEAC", "Cameroon, Central African Republic, Congo, Chad, Equatorial Guinea, Gabon",
	"XAG", "961", "0", "Silver (one troy ounce)", "",
	"XAU", "959", "0", "Gold (one troy ounce)", "",
	"XBA", "955", "0", "European Composite Unit (EURCO) (bond market unit)", "",
	"XBB", "956", "0", "European Monetary Unit (E.M.U.-6) (bond market unit)", "",
	"XBC", "957", "0", "European Unit of Account 9 (E.U.A.-9) (bond market unit)", "",
	"XBD", "958", "0", "European Unit of Account 17 (E.U.A.-17) (bond market unit)", "",
	"XCD", "951", "2", "East Caribbean dollar", "Anguilla, Antigua and Barbuda, Dominica, Grenada, Montserrat, Saint Kitts and Nevis, Saint Lucia, Saint Vincent and the Grenadines",
	"XDR", "960", "0", "Special Drawing Rights", "International Monetary Fund",
	"XFU", "Nil", "0", "UIC franc (special settlement currency)", "International Union of Railways",
	"XOF", "952", "0", "CFA Franc BCEAO", "Benin, Burkina Faso, C�te d\'Ivoire, Guinea-Bissau, Mali, Niger, Senegal, Togo",
	"XPD", "964", "0", "Palladium (one troy ounce)", "",
	"XPF", "953", "0", "CFP franc", "French Polynesia, New Caledonia, Wallis and Futuna",
	"XPT", "962", "0", "Platinum (one troy ounce)", "",
	"XTS", "963", "0", "Code reserved for testing purposes", "",
	"XXX", "999", "0", "No currency", "",
	"YER", "886", "0", "Yemeni rial", "Yemen",
	"ZAR", "710", "2", "South African rand", "South Africa",
	"ZMK", "894", "0", "Kwacha", "Zambia",
	"ZWD", "716", "2", "Zimbabwe dollar", "Zimbabwe",
	"US500", "996", "2", "US 500 index", "",
	"AUS200", "995", "1", "ASX 200 index", "",
	"GER30", "994", "1", "GER 30 index", "",
	"XTI", "991", "2", "XTI OIL", "",
	"BTC", "992", "2", "BitCoin", "",
	"ETH", "899", "2", "EthenetCoin", "",
	"USTEC", "993", "2", "US 100 Tech index", "",
	"NAS100", "988", "2", "US 100 Tech index", "",
	"SpotCrude", "989", "3", "Spot Crude", "",
};

/* ---------------------------------------------------------------------------------------------------------------------------------------------*/

AsirikuyReturnCode parseSymbol(const char* pSymbol, char* pPrefix, char* pBaseCurrency, char* pSeparator, char* pQuoteCurrency, char* pSuffix)
{
	int i=0, offset=-1, baseCurrencyOffset = -1, quoteCurrencyOffset = -1, prefixLength, suffixLength, separatorLength;

	//TODO:
	//Override US500, NAS100 and SportCrude 
	//if (strcmp(pSymbol, "NAS100") == 0 || strcmp(pSymbol, "US500") == 0 || strcmp(pSymbol, "SpotCrude") == 0) {
	//	strcat(pSymbol, "USD\n\n");
	//	pantheios_logputs(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"convert Symbol. pSymbol = %s", pSymbol);
	//}

  /* If any pointers are NULL return now to avoid a memory access violation */
  if(pSymbol == NULL)
  {
    fprintf(stderr, "[CRITICAL] parseSymbol() failed. pSymbol = NULL\n\n");
    return NULL_POINTER;
  }
  else
  {
    fprintf(stderr, "[DEBUG] parseSymbol() pSymbol = %s", pSymbol);
  }
  if(pPrefix == NULL)
  {
    fprintf(stderr, "[CRITICAL] parseSymbol() failed. pPrefix = NULL\n\n");
    return NULL_POINTER;
  }
  if(pBaseCurrency == NULL)
  {
    fprintf(stderr, "[CRITICAL] parseSymbol() failed. pBaseCurrency = NULL\n\n");
    return NULL_POINTER;
  }
  if(pSeparator == NULL)
  {
    fprintf(stderr, "[CRITICAL] parseSymbol() failed. pSeparator = NULL\n\n");
    return NULL_POINTER;
  }
  if(pQuoteCurrency == NULL)
  {
    fprintf(stderr, "[CRITICAL] parseSymbol() failed. pQuoteCurrency = NULL\n\n");
    return NULL_POINTER;
  }
  if(pSuffix == NULL)
  {
    fprintf(stderr, "[CRITICAL] parseSymbol() failed. pSuffix = NULL\n\n");
    return NULL_POINTER;
  }

	for(i = 0; i < TOTAL_CURRENCIES; i++)
	{
		offset = strstr(pSymbol, g_currencies[i][CURRENCY_CODE]) - pSymbol; 

		if(offset >= 0 && offset < (int)strlen(pSymbol))
		{
			if(baseCurrencyOffset == -1)
			{
        /* Assume for now that this offset is the base currency */
				baseCurrencyOffset = offset;
				strcpy(pBaseCurrency, g_currencies[i][CURRENCY_CODE]);
			}
			else if(quoteCurrencyOffset == -1)
			{
				if(offset < baseCurrencyOffset)
				{
          /* Found the base currency. The first offset must have been the quote currency. Swap them. */
					quoteCurrencyOffset = baseCurrencyOffset;
					baseCurrencyOffset  = offset;
					strcpy(pQuoteCurrency, pBaseCurrency);
					strcpy(pBaseCurrency, g_currencies[i][CURRENCY_CODE]);
				}
				else
				{
          /* Found the quote currency offset */
					quoteCurrencyOffset = offset;
					strcpy(pQuoteCurrency, g_currencies[i][CURRENCY_CODE]);
				}

        /* baseCurrencyOffset and quoteCurrencyOffset have both been found. */
				break;
			}
		}
	}

  /* Did we fail to find a second currency offset? If so exit now to avoid undefined behaviour */
	if(quoteCurrencyOffset < 0)
  {
    fprintf(stderr, "[ERROR] parseSymbol() failed. Unknown symbol. pSymbol = %s", pSymbol);
		return UNKNOWN_SYMBOL;
  }

	prefixLength    = baseCurrencyOffset;
	suffixLength    = strlen(pSymbol) - strlen(pQuoteCurrency) - quoteCurrencyOffset;
	separatorLength = quoteCurrencyOffset - strlen(pBaseCurrency) - baseCurrencyOffset;

  /* If any of these values are below 0 it means pSymbol is too short. Exit now to avoid a potential memory access violation */
  if(prefixLength < 0 || suffixLength < 0 || separatorLength < 0)
  {
    fprintf(stderr, "[ERROR] parseSymbol() failed. Symbol too short. pSymbol = %s", pSymbol);
		return SYMBOL_TOO_SHORT;
  }

	memcpy(pPrefix, pSymbol, baseCurrencyOffset);
	pPrefix[prefixLength] = '\0';

	memcpy(pSuffix, pSymbol + quoteCurrencyOffset + strlen(pQuoteCurrency), suffixLength);
	pSuffix[suffixLength] = '\0';

	memcpy(pSeparator, pSymbol + baseCurrencyOffset + strlen(pBaseCurrency), separatorLength);
	pSeparator[separatorLength] = '\0';

  fprintf(stderr, "[DEBUG] parseSymbol() succeeded. pPrefix = %s, pBaseCurrency = %s, pSeparator = %s, pQuoteCurrency = %s, pSuffix = %s\n", 
    pPrefix, pBaseCurrency, pSeparator, pQuoteCurrency, pSuffix);

	return SUCCESS;
}
/* ---------------------------------------------------------------------------------------------------------------------------------------------*/

AsirikuyReturnCode normalizeSymbol(char* pSymbol)
{
  char prefix[PRE_SEP_SUF_SIZE], baseCurrency[CURRENCY_SIZE], separator[PRE_SEP_SUF_SIZE], quoteCurrency[CURRENCY_SIZE], suffix[PRE_SEP_SUF_SIZE];

  /* If pSymbol is NULL exit now to avoid a memory access violation */
  if(pSymbol == NULL)
  {
    fprintf(stderr, "[CRITICAL] normalizeSymbol() failed. pSymbol = NULL\n\n");
    return NULL_POINTER;
  }

  if(SUCCESS != parseSymbol(pSymbol, prefix, baseCurrency, separator, quoteCurrency, suffix))
  {
    fprintf(stderr, "[ERROR] normalizeSymbol() failed. Unable to parse symbol.\n\n");
    return UNKNOWN_SYMBOL;
  }

  /* Only keep the base and quote currencies. Discard anything else */
  strncpy(pSymbol, baseCurrency, 3);
  strncpy(&pSymbol[3], quoteCurrency, 3);
  pSymbol[6] = '\0';

  fprintf(stderr, "[DEBUG] normalizeSymbol() succeeded. pSymbol = %s", pSymbol);
  return SUCCESS;
}
/* ---------------------------------------------------------------------------------------------------------------------------------------------*/

AsirikuyReturnCode normalizeCurrency(char* pCurrency)
{
  int i;

  /* If pCurrency is NULL exit now to avoid a memory access violation */
  if(pCurrency == NULL)
  {
    fprintf(stderr, "[CRITICAL] normalizeCurrency() failed. pCurrency = NULL\n\n");
    return NULL_POINTER;
  }
  //g_currencies[TOTAL_CURRENCIES][TOTAL_CURRENCY_INFO_INDEXES][CURRENCY_INFO_STRING_SIZE]
  for(i = 0; i < TOTAL_CURRENCIES; i++)
	{
		if(strstr(pCurrency, g_currencies[i][0]) != NULL)
		{
      strcpy(pCurrency, g_currencies[i][0]);
      fprintf(stderr, "[DEBUG] normalizeCurrency() succeeded. pCurrency = %s", pCurrency);
      return SUCCESS;
    }
  }

  fprintf(stderr, "[WARNING] normalizeCurrency() %s is not a recognized currency, defaulting to \"USD\". This may occur when using cent accounts on some brokers.", pCurrency);
  strcpy(pCurrency, "USD\n\n");
  return SUCCESS;
}
/* ---------------------------------------------------------------------------------------------------------------------------------------------*/

AsirikuyReturnCode getConversionSymbols(const char* pSymbol, char* pAccountCurrency, char* pBaseConversionSymbol, char* pQuoteConversionSymbol)
{
  AsirikuyReturnCode returnCode;
	int i, foundBaseConversionSymbol = FALSE, foundQuoteConversionSymbol = FALSE;
  char prefix[PRE_SEP_SUF_SIZE], baseCurrency[CURRENCY_SIZE], separator[PRE_SEP_SUF_SIZE], quoteCurrency[CURRENCY_SIZE], suffix[PRE_SEP_SUF_SIZE];

  /* If any pointers are NULL return now to avoid a memory access violation */
  if(pSymbol == NULL)
  {
    fprintf(stderr, "[CRITICAL] getConversionSymbols() failed. pSymbol = NULL\n\n");
    return NULL_POINTER;
  }
  if(pAccountCurrency == NULL)
  {
    fprintf(stderr, "[CRITICAL] getConversionSymbols() failed. pAccountCurrency = NULL\n\n");
    return NULL_POINTER;
  }
  if(pBaseConversionSymbol == NULL)
  {
    fprintf(stderr, "[CRITICAL] getConversionSymbols() failed. pBaseConversionSymbol = NULL\n\n");
    return NULL_POINTER;
  }
  if(pQuoteConversionSymbol == NULL)
  {
    fprintf(stderr, "[CRITICAL] getConversionSymbols() failed. pQuoteConversionSymbol = NULL\n\n");
    return NULL_POINTER;
  }
  
  returnCode = normalizeCurrency(pAccountCurrency);
  if(returnCode != SUCCESS)
  {
    return returnCode;
  }

  returnCode = parseSymbol(pSymbol, prefix, baseCurrency, separator, quoteCurrency, suffix);
  if(returnCode != SUCCESS)
  {
    return returnCode;
  }

	for(i = 0; i < TOTAL_CURRENCY_PAIRS; i++)
	{
		if(strcmp(g_currencyPairs[i][BASE_CURRENCY], baseCurrency) == 0 && strcmp(g_currencyPairs[i][QUOTE_CURRENCY], pAccountCurrency) == 0)
		{
			strcpy(pBaseConversionSymbol, "\n\n");
			strcat(pBaseConversionSymbol, prefix);
			strcat(pBaseConversionSymbol, baseCurrency);
			strcat(pBaseConversionSymbol, separator);
			strcat(pBaseConversionSymbol, pAccountCurrency);
			strcat(pBaseConversionSymbol, suffix);
			foundBaseConversionSymbol = TRUE;
		}

		if(strcmp(g_currencyPairs[i][BASE_CURRENCY], pAccountCurrency) == 0 && strcmp(g_currencyPairs[i][QUOTE_CURRENCY], baseCurrency) == 0)
		{
			strcpy(pBaseConversionSymbol, "\n\n");
			strcat(pBaseConversionSymbol, prefix);
			strcat(pBaseConversionSymbol, pAccountCurrency);
			strcat(pBaseConversionSymbol, separator);
			strcat(pBaseConversionSymbol, baseCurrency);
			strcat(pBaseConversionSymbol, suffix);
			foundBaseConversionSymbol = TRUE;
		}

		if(strcmp(g_currencyPairs[i][BASE_CURRENCY], pAccountCurrency) == 0 && strcmp(g_currencyPairs[i][QUOTE_CURRENCY], quoteCurrency) == 0)
		{
			strcpy(pQuoteConversionSymbol, "\n\n");
			strcat(pQuoteConversionSymbol, prefix);
			strcat(pQuoteConversionSymbol, pAccountCurrency);
			strcat(pQuoteConversionSymbol, separator);
			strcat(pQuoteConversionSymbol, quoteCurrency);
			strcat(pQuoteConversionSymbol, suffix);
			foundQuoteConversionSymbol = TRUE;
		}

		if(strcmp(g_currencyPairs[i][BASE_CURRENCY], quoteCurrency) == 0 && strcmp(g_currencyPairs[i][QUOTE_CURRENCY], pAccountCurrency) == 0)
		{
			strcpy(pQuoteConversionSymbol, "\n\n");
			strcat(pQuoteConversionSymbol, prefix);
			strcat(pQuoteConversionSymbol, quoteCurrency);
			strcat(pQuoteConversionSymbol, separator);
			strcat(pQuoteConversionSymbol, pAccountCurrency);
			strcat(pQuoteConversionSymbol, suffix);
			foundQuoteConversionSymbol = TRUE;
		}

		if(foundBaseConversionSymbol && foundQuoteConversionSymbol)
    {
      /* Both symbols have been found. Exit early */
      fprintf(stderr, "[DEBUG] getConversionSymbols() succeeded. pBaseConversionSymbol = %s, pQuoteConversionSymbol = %s", pBaseConversionSymbol, pQuoteConversionSymbol);
			return SUCCESS;
    }
	}

	if(foundBaseConversionSymbol || foundQuoteConversionSymbol)
	{
    /* Only one of the conversion symbols needs to be found. Exit successful. */
    fprintf(stderr, "[DEBUG] getConversionSymbols() succeeded but only 1 conversion symbol was found. pBaseConversionSymbol = %s, pQuoteConversionSymbol = %s", pBaseConversionSymbol, pQuoteConversionSymbol);
	  return SUCCESS;
  }

  /* Failed to find any conversion symbols. Exit with failure. */
  fprintf(stderr, "[ERROR] getConversionSymbols() failed. Unable to find any conversion symbols.\n\n");
	return NO_CONVERSION_SYMBOLS;
}
/* ---------------------------------------------------------------------------------------------------------------------------------------------*/

AsirikuyReturnCode getCurrencyPairPrefix(const char* pSymbol, char* pPrefix)
{
	char prefix[PRE_SEP_SUF_SIZE], baseCurrency[CURRENCY_SIZE], separator[PRE_SEP_SUF_SIZE], quoteCurrency[CURRENCY_SIZE], suffix[PRE_SEP_SUF_SIZE];

  /* If any pointers are NULL return now to avoid a memory access violation */
  if(pSymbol == NULL)
  {
    fprintf(stderr, "[CRITICAL] getCurrencyPairPrefix() failed. pSymbol = NULL\n\n");
    return NULL_POINTER;
  }
  if(pPrefix == NULL)
  {
    fprintf(stderr, "[CRITICAL] getCurrencyPairPrefix() failed. pPrefix = NULL\n\n");
    return NULL_POINTER;
  }

  if(SUCCESS != parseSymbol(pSymbol, prefix, baseCurrency, separator, quoteCurrency, suffix))
  {
    fprintf(stderr, "[ERROR] getCurrencyPairPrefix() failed. Unable to parse symbol.\n\n");
    return UNKNOWN_SYMBOL;
  }

  strcpy(pPrefix, prefix);
  fprintf(stderr, "[DEBUG] getCurrencyPairPrefix() succeeded. pSymbol = %s, pPrefix = %s", pSymbol, pPrefix);
  return SUCCESS;
}
/* ---------------------------------------------------------------------------------------------------------------------------------------------*/

AsirikuyReturnCode getBaseCurrency(const char* pSymbol, char* pBaseCurrency)
{
	char prefix[PRE_SEP_SUF_SIZE], baseCurrency[CURRENCY_SIZE], separator[PRE_SEP_SUF_SIZE], quoteCurrency[CURRENCY_SIZE], suffix[PRE_SEP_SUF_SIZE];

  /* If any pointers are NULL return now to avoid a memory access violation */
  if(pSymbol == NULL)
  {
    fprintf(stderr, "[CRITICAL] getBaseCurrency() failed. pSymbol = NULL\n\n");
    return NULL_POINTER;
  }
  if(pBaseCurrency == NULL)
  {
    fprintf(stderr, "[CRITICAL] getBaseCurrency() failed. pBaseCurrency = NULL\n\n");
    return NULL_POINTER;
  }

  if(SUCCESS != parseSymbol(pSymbol, prefix, baseCurrency, separator, quoteCurrency, suffix))
  {
    fprintf(stderr, "[ERROR] getBaseCurrency() failed. Unable to parse symbol.\n\n");
    return UNKNOWN_SYMBOL;
  }

  strcpy(pBaseCurrency, baseCurrency);
  fprintf(stderr, "[DEBUG] getBaseCurrency() succeeded. pSymbol = %s, pBaseCurrency = %s", pSymbol, pBaseCurrency);
  return SUCCESS;
}
/* ---------------------------------------------------------------------------------------------------------------------------------------------*/

AsirikuyReturnCode getCurrencyPairSeparator(const char* pSymbol, char* pSeparator)
{
	char prefix[PRE_SEP_SUF_SIZE], baseCurrency[CURRENCY_SIZE], separator[PRE_SEP_SUF_SIZE], quoteCurrency[CURRENCY_SIZE], suffix[PRE_SEP_SUF_SIZE];

  /* If any pointers are NULL return now to avoid a memory access violation */
  if(pSymbol == NULL)
  {
    fprintf(stderr, "[CRITICAL] getCurrencyPairSeparator() failed. pSymbol = NULL\n\n");
    return NULL_POINTER;
  }
  if(pSeparator == NULL)
  {
    fprintf(stderr, "[CRITICAL] getCurrencyPairSeparator() failed. pSeparator = NULL\n\n");
    return NULL_POINTER;
  }

  if(SUCCESS != parseSymbol(pSymbol, prefix, baseCurrency, separator, quoteCurrency, suffix))
  {
    fprintf(stderr, "[ERROR] getCurrencyPairSeparator() failed. Unable to parse symbol.\n\n");
    return UNKNOWN_SYMBOL;
  }

	strcpy(pSeparator, separator);
	fprintf(stderr, "[DEBUG] getCurrencyPairSeparator() succeeded. pSymbol = %s, pSeparator = %s", pSymbol, pSeparator);
  return SUCCESS;
}
/* ---------------------------------------------------------------------------------------------------------------------------------------------*/

AsirikuyReturnCode getQuoteCurrency(const char* pSymbol, char* pQuoteCurrency)
{
	char prefix[PRE_SEP_SUF_SIZE], baseCurrency[CURRENCY_SIZE], separator[PRE_SEP_SUF_SIZE], quoteCurrency[CURRENCY_SIZE], suffix[PRE_SEP_SUF_SIZE];

  /* If any pointers are NULL return now to avoid a memory access violation */
  if(pSymbol == NULL)
  {
    fprintf(stderr, "[CRITICAL] getQuoteCurrency() failed. pSymbol = NULL\n\n");
    return NULL_POINTER;
  }
  if(pQuoteCurrency == NULL)
  {
    fprintf(stderr, "[CRITICAL] getQuoteCurrency() failed. pQuoteCurrency = NULL\n\n");
    return NULL_POINTER;
  }

  if(SUCCESS != parseSymbol(pSymbol, prefix, baseCurrency, separator, quoteCurrency, suffix))
  {
    fprintf(stderr, "[ERROR] getQuoteCurrency() failed. Unable to parse symbol.\n\n");
    return UNKNOWN_SYMBOL;
  }

  strcpy(pQuoteCurrency, quoteCurrency);
	fprintf(stderr, "[DEBUG] getQuoteCurrency() succeeded. pSymbol = %s, pQuoteCurrency = %s", pSymbol, pQuoteCurrency);
  return SUCCESS;
}
/* ---------------------------------------------------------------------------------------------------------------------------------------------*/

AsirikuyReturnCode getCurrencyPairSuffix(const char* pSymbol, char* pSuffix)
{
	char prefix[PRE_SEP_SUF_SIZE], baseCurrency[CURRENCY_SIZE], separator[PRE_SEP_SUF_SIZE], quoteCurrency[CURRENCY_SIZE], suffix[PRE_SEP_SUF_SIZE];

  /* If any pointers are NULL return now to avoid a memory access violation */
  if(pSymbol == NULL)
  {
    fprintf(stderr, "[CRITICAL] getCurrencyPairSuffix() failed. pSymbol = NULL\n\n");
    return NULL_POINTER;
  }
  if(pSuffix == NULL)
  {
    fprintf(stderr, "[CRITICAL] getCurrencyPairSuffix() failed. pSuffix = NULL\n\n");
    return NULL_POINTER;
  }

  if(SUCCESS != parseSymbol(pSymbol, prefix, baseCurrency, separator, quoteCurrency, suffix))
  {
    fprintf(stderr, "[ERROR] getCurrencyPairSuffix() failed. Unable to parse symbol.\n\n");
    return UNKNOWN_SYMBOL;
  }

	strcpy(pSuffix, suffix);
	fprintf(stderr, "[DEBUG] getCurrencyPairSuffix() succeeded. pSymbol = %s, pSuffix = %s", pSymbol, pSuffix);
  return SUCCESS;
}
/* ---------------------------------------------------------------------------------------------------------------------------------------------*/

AsirikuyReturnCode getCurrencyInfo(char* pCurrencyCode, char* pCurrencyNumber, char* pDigitsAfterDecimal, char* pCurrencyName, char* pCurrencyLocations)
{
  AsirikuyReturnCode returnCode;
	int i;

  /* If any pointers are NULL return now to avoid a memory access violation */
  if(pCurrencyCode == NULL)
  {
    fprintf(stderr, "[CRITICAL] getCurrencyInfo() failed. pCurrencyCode = NULL\n\n");
    return NULL_POINTER;
  }
  if(pCurrencyNumber == NULL)
  {
    fprintf(stderr, "[CRITICAL] getCurrencyInfo() failed. pCurrencyNumber = NULL\n\n");
    return NULL_POINTER;
  }
  if(pDigitsAfterDecimal == NULL)
  {
    fprintf(stderr, "[CRITICAL] getCurrencyInfo() failed. pDigitsAfterDecimal = NULL\n\n");
    return NULL_POINTER;
  }
  if(pCurrencyName == NULL)
  {
    fprintf(stderr, "[CRITICAL] getCurrencyInfo() failed. pCurrencyName = NULL\n\n");
    return NULL_POINTER;
  }
  if(pCurrencyLocations == NULL)
  {
    fprintf(stderr, "[CRITICAL] getCurrencyInfo() failed. pCurrencyLocations = NULL\n\n");
    return NULL_POINTER;
  }
  
  returnCode = normalizeCurrency(pCurrencyCode);
  if(returnCode != SUCCESS)
  {
    return returnCode;
  }

	for(i = 0; i < TOTAL_CURRENCIES; i++)
	{
		if(strcmp(g_currencies[i][CURRENCY_CODE], pCurrencyCode) == 0)
		{
			strcpy(pCurrencyNumber, g_currencies[i][CURRENCY_NUMBER]);
			strcpy(pDigitsAfterDecimal, g_currencies[i][DIGITS_AFTER_DECIMAL]);
			strcpy(pCurrencyName, g_currencies[i][CURRENCY_NAME]);
			strcpy(pCurrencyLocations, g_currencies[i][CURRENCY_LOCATIONS]);

			fprintf(stderr, "[DEBUG] getCurrencyInfo() succeeded. pCurrencyCode = %s, pCurrencyNumber = %s, pDigitsAfterDecimal = %s, pCurrencyName = %s, pCurrencyLocations = %s\n", 
        pCurrencyCode, pCurrencyNumber, pDigitsAfterDecimal, pCurrencyName, pCurrencyLocations);

      return SUCCESS;
		}
	}

	fprintf(stderr, "[ERROR] getCurrencyInfo() failed. Invalid currency. pCurrencyCode = %s", pCurrencyCode);
  return INVALID_CURRENCY;
}
/* ---------------------------------------------------------------------------------------------------------------------------------------------*/

AsirikuyReturnCode getCurrencyNumber(char* pCurrencyCode, char* pCurrencyNumber)
{
  AsirikuyReturnCode returnCode;
	int i;
  
  /* If any pointers are NULL return now to avoid a memory access violation */
  if(pCurrencyCode == NULL)
  {
    fprintf(stderr, "[CRITICAL] getCurrencyNumber() failed. pCurrencyCode = NULL\n\n");
    return NULL_POINTER;
  }
  if(pCurrencyNumber == NULL)
  {
    fprintf(stderr, "[CRITICAL] getCurrencyNumber() failed. pCurrencyNumber = NULL\n\n");
    return NULL_POINTER;
  }
  
  returnCode = normalizeCurrency(pCurrencyCode);
  if(returnCode != SUCCESS)
  {
    return returnCode;
  }

	for(i = 0; i < TOTAL_CURRENCIES; i++)
	{
		if(strcmp(g_currencies[i][CURRENCY_CODE], pCurrencyCode) == 0)
		{
			strcpy(pCurrencyNumber, g_currencies[i][CURRENCY_NUMBER]);

			fprintf(stderr, "[DEBUG] getCurrencyNumber() succeeded. pCurrencyCode = %s, pCurrencyNumber = %s", pCurrencyCode, pCurrencyNumber);
      return SUCCESS;
		}
	}

	fprintf(stderr, "[ERROR] getCurrencyNumber() failed. Invalid currency. pCurrencyCode = %s", pCurrencyCode);
  return INVALID_CURRENCY;
}
/* ---------------------------------------------------------------------------------------------------------------------------------------------*/

AsirikuyReturnCode getNumDigitsAfterDecimal(char* pCurrencyCode, char* pDigitsAfterDecimal)
{
  AsirikuyReturnCode returnCode;
	int i;
  
  /* If any pointers are NULL return now to avoid a memory access violation */
  if(pCurrencyCode == NULL)
  {
    fprintf(stderr, "[CRITICAL] getNumDigitsAfterDecimal() failed. pCurrencyCode = NULL\n\n");
    return NULL_POINTER;
  }
  if(pDigitsAfterDecimal == NULL)
  {
    fprintf(stderr, "[CRITICAL] getNumDigitsAfterDecimal() failed. pDigitsAfterDecimal = NULL\n\n");
    return NULL_POINTER;
  }
  
  returnCode = normalizeCurrency(pCurrencyCode);
  if(returnCode != SUCCESS)
  {
    return returnCode;
  }

	for(i = 0; i < TOTAL_CURRENCIES; i++)
	{
		if(strcmp(g_currencies[i][CURRENCY_CODE], pCurrencyCode) == 0)
		{
			strcpy(pDigitsAfterDecimal, g_currencies[i][DIGITS_AFTER_DECIMAL]);
			
			fprintf(stderr, "[DEBUG] getNumDigitsAfterDecimal() succeeded. pCurrencyCode = %s, pDigitsAfterDecimal = %s", pCurrencyCode, pDigitsAfterDecimal);
      return SUCCESS;
		}
	}

	fprintf(stderr, "[ERROR] getNumDigitsAfterDecimal() failed. Invalid currency. pCurrencyCode = %s", pCurrencyCode);
  return INVALID_CURRENCY;
}
/* ---------------------------------------------------------------------------------------------------------------------------------------------*/

AsirikuyReturnCode getCurrencyName(char* pCurrencyCode, char* pCurrencyName)
{
  AsirikuyReturnCode returnCode;
	int i;
  
  /* If any pointers are NULL return now to avoid a memory access violation */
  if(pCurrencyCode == NULL)
  {
    fprintf(stderr, "[CRITICAL] getCurrencyName() failed. pCurrencyCode = NULL\n\n");
    return NULL_POINTER;
  }
  if(pCurrencyName == NULL)
  {
    fprintf(stderr, "[CRITICAL] getCurrencyName() failed. pCurrencyName = NULL\n\n");
    return NULL_POINTER;
  }
  
  returnCode = normalizeCurrency(pCurrencyCode);
  if(returnCode != SUCCESS)
  {
    return returnCode;
  }

	for(i = 0; i < TOTAL_CURRENCIES; i++)
	{
		if(strcmp(g_currencies[i][CURRENCY_CODE], pCurrencyCode) == 0)
		{
			strcpy(pCurrencyName, g_currencies[i][CURRENCY_NAME]);
			
			fprintf(stderr, "[DEBUG] getCurrencyName() succeeded. pCurrencyCode = %s, pCurrencyName = %s", pCurrencyCode, pCurrencyName);
      return SUCCESS;
		}
	}

	fprintf(stderr, "[ERROR] getCurrencyName() failed. Invalid currency. pCurrencyCode = %s", pCurrencyCode);
  return INVALID_CURRENCY;
}
/* ---------------------------------------------------------------------------------------------------------------------------------------------*/

AsirikuyReturnCode getCurrencyLocations(char* pCurrencyCode, char* pCurrencyLocations)
{
  AsirikuyReturnCode returnCode;
	int i;
  
  /* If any pointers are NULL return now to avoid a memory access violation */
  if(pCurrencyCode == NULL)
  {
    fprintf(stderr, "[CRITICAL] getCurrencyLocations() failed. pCurrencyCode = NULL\n\n");
    return NULL_POINTER;
  }
  if(pCurrencyLocations == NULL)
  {
    fprintf(stderr, "[CRITICAL] getCurrencyLocations() failed. pCurrencyLocations = NULL\n\n");
    return NULL_POINTER;
  }

  returnCode = normalizeCurrency(pCurrencyCode);
  if(returnCode != SUCCESS)
  {
    return returnCode;
  }

	for(i = 0; i < TOTAL_CURRENCIES; i++)
	{
		if(strcmp(g_currencies[i][CURRENCY_CODE], pCurrencyCode) == 0)
		{
			strcpy(pCurrencyLocations, g_currencies[i][CURRENCY_LOCATIONS]);
			
			fprintf(stderr, "[DEBUG] getCurrencyLocations() succeeded. pCurrencyCode = %s, pCurrencyLocations = %s", pCurrencyCode, pCurrencyLocations);
      return SUCCESS;
		}
	}

	fprintf(stderr, "[ERROR] getCurrencyLocations() failed. Invalid currency. pCurrencyCode = %s", pCurrencyCode);
  return INVALID_CURRENCY;
}
/* ---------------------------------------------------------------------------------------------------------------------------------------------*/