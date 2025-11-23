from colorama import init, Fore, Back, Style
import logging, argparse, ctypes, sys, os
import logging.handlers
from time import strftime
from include.asirikuy import *
from include.misc import *
import datetime as dt
import time, calendar
import subprocess
from winnt import MAXBYTE

def main():

    parser = argparse.ArgumentParser()
    parser.add_argument('-c', '--config-file')
    parser.add_argument('-v','--version', nargs='*')
    args = parser.parse_args()

    if args.version != None:
        version()
        quit()

    #Colorama Initialization
    init(autoreset = True)
    
    #Paths
    if args.config_file:
        configFilePath = args.config_file
    else:
        configFilePath = './config/checker.config'

    config = readConfigFile(configFilePath)    

    monitoringInterval = float(config.get('general', 'monitoringInterval'))
    openDay =   int(config.get('general', 'weekOpenDay'))
    openHour =  int(config.get('general', 'weekOpenHour'))
    closeDay =  int(config.get('general', 'weekCloseDay'))
    closeHour = int(config.get('general', 'weekCloseHour'))
    useEmail = int(config.get('general', 'useEmail'))
    fromEmail = config.get('general', 'fromEmail')
    toEmail = config.get('general', 'toEmail')
    emailLogin= config.get('general', 'emailLogin')
    emailPassword= config.get('general', 'emailPassword')
    smtpServer= config.get('general', 'smtpServer')
    
    accountSections = map(str.strip, config.get('accounts', 'accounts').split(','))
    
    LOG_FILENAME = "./log/monitor.log"
    
    logging.basicConfig(filename=LOG_FILENAME,
                            filemode='a',
                            format='%(asctime)s,%(msecs)d %(name)s %(levelname)s %(message)s',
                            datefmt='%H:%M:%S',
                            level=logging.ERROR)
    
    logger = logging.getLogger('Monitor')
    #logger.setLevel(logging.DEBUG)
    
    #handler = logging.handlers.RotatingFileHandler(LOG_FILENAME, maxBytes=20, backupCount=100)

    # create a logging format
    #formatter = logging.Formatter('%(asctime)s,%(msecs)d %(name)s %(levelname)s %(message)s')
    #handler.setFormatter(formatter)

    #logger.addHandler(handler)

    logger.info("Monitor staring")


    i = 0

    lastError = []

    for accountSection in accountSections:
        lastError.append("")

    while True:
        
        clearScreen()

        localTime = dt.datetime.fromtimestamp(mktime(time.localtime(time.time())))
        currentGmTime = gmtime()
        #hour = currentGmTime.tm_hour
        #day = currentGmTime.tm_wday

        hour = time.localtime().tm_hour
        day = time.localtime().tm_wday

        print( "Local Hour = %s" % hour )
        print( "Loca Day = %s" % day )

        
        
        if ((hour > openHour) and (day == openDay)) or ((hour < closeHour) and (day == closeDay)) or ((day > openDay) and (day < closeDay)): 

            i = 0
        
            for accountSection in accountSections:
                path = config.get(accountSection, 'path')
                frontEnd = config.get(accountSection, 'frontend')
                accountNumber = config.get(accountSection, 'accountNumber')

                #check the heartbeat
                import os
        
                if frontEnd == "MT4":
                    os.chdir(path + '/MQL4/Files')
                else:
                    os.chdir(path + '/tmp')

                for files in os.listdir("."):
                                        
                    if files.endswith(".hb"):
                        fp = open(files,'r')
                        data = fp.readlines()
                        fp.close()
                        date = dt.datetime.strptime(data[1].strip(),"%d/%m/%y %H:%M")
                        output = "Checking %s..." % files
                        print (output)
                        logger.info(output)
                        
                        utc_datetime = dt.datetime.utcnow()                        
                        diff = abs(utc_datetime-date).total_seconds()
                        output = 'Current Time: {0}, Current UTC Time {1}, Last heart-beat time: {2}, Difference in {3} secs: {4}'.format(localTime, utc_datetime,date, monitoringInterval*2.5, diff)
                        print output
                        logger.info(output)
                        if diff > monitoringInterval*2.5:
                            output = "Heart-Beat problem for instance %s" % files
                            print (Style.BRIGHT + Fore.RED + output)
                            logger.error(output)
                            
                            os.unlink(files)
                            if frontEnd == "MT4":
                                logger.error("Killing MT4")
                                subprocess.call("taskkill /f /im terminal.exe")
                            
                            if useEmail == 1:
                                output = "Sending email message about heart beat..."
                                print output
                                logger.error(output)  
                                sendemail(fromEmail, toEmail, "", accountSection + " error message", ("Heart-Beat problem for instance %s, system not updating for more than %s secs" % files, str(monitoringInterval*2.5) ), emailLogin, emailPassword, smtpServer)
                            break                               
                #log checking step

                if frontEnd == "MT4":
                    fname =  path + '/MQL4/Logs/AsirikuyFramework.log'
                else:
                    fname = path + '/log/' + str(accountNumber) + 'AsirikuyFramework.log'
                    
                if os.path.exists(fname):
                    file = open(fname,'r')
                
                    data = file.readlines() 
                    file.close()

                    #FMT = '%H:%M:%S.%f'
                    #lastLine = data[len(data)-1]
                    #lastTime = dt.datetime.strptime(lastLine.split(' ')[3], FMT);
                    
                    # look back the last 6 lines
                    # if they are within 10 seconds, will check error
                    #for index in range(len(data)-10,len(data)):
                        
                        #line = data[index]                        
                        #tdelta = lastTime - dt.datetime.strptime(line.split(' ')[3], FMT)
                        #print tdelta.total_seconds()                   
                        #print line

                    line = data[len(data) -1]
                    #print line
                    #if tdelta.total_seconds() < 5 and ( "Error" in line  or "Emergency" in line or "Critical" in line ):
                    if "Error" in line  or "Emergency" in line or "Critical" in line :                        
                        output = "Error detected on account %s" % accountSection
                        print (Style.BRIGHT + Fore.RED + output)
                        logger.error(output)
                        print line
                        logger.error(line)
                        
                        if frontEnd == "MT4" and "Error" in line:
                            logger.error("Killing MT4")                        
                            subprocess.call("taskkill /f /im terminal.exe")
                        
                        if useEmail == 1 and lastError[i] != data[len(data)-1]:                        
                            output =  "Sending email message about error..."
                            logger.error(output)
                            lastError[i] = data[len(data)-1]
                            sendemail(fromEmail, toEmail, "", accountSection + " error message", data[len(data)-1], emailLogin, emailPassword, smtpServer)
                        break    

                i += 1
                
        output = 'Last execution %s, sleeping for %s secs.' % (strftime("%a, %d %b %Y %X"), monitoringInterval)
        print (Style.BRIGHT + Back.GREEN + output).ljust(100)
        logger.info(output)
        sleep(monitoringInterval)
        
def version():
    print "Asirikuy Monitor v0.05"

    
        
if __name__ == "__main__": main()
