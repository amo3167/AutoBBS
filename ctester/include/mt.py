import re, configparser
from include.asirikuy import *
from collections import OrderedDict

def readSetFile(filePath):
    try:
        config = configparser.RawConfigParser()
        config.optionxform = str
        config.read_file(ModifiedSetFile(open(filePath)))
        return config
    except IOError as e:
        print(e)
        return False
    except:
        return False
    
class ModifiedSetFile(object):
    def __init__(self, fp):
        self.fp = fp
        self.sechead = '[main]\n'
        
    def readline(self):
        #Add [main] section
        if self.sechead:
            try: return self.sechead
            finally: self.sechead = None
        else: 
            line = self.fp.readline()
        #Add [additional] section
        return re.sub('^SECTION_3.*$', '[additional]', line)
    
    def __iter__(self):
        # Make the object iterable for Python 3 configparser
        if self.sechead:
            yield self.sechead
            self.sechead = None
        for line in self.fp:
            yield re.sub('^SECTION_3.*$', '[additional]', line)

class MT4Set():
    def __init__(self, filePath):
        #Read set file
        self.content = readSetFile(filePath)
        if not self.content: return
        self.sections = self.content._sections
        self.mainParams = {}
        self.additionalParams = {}
        self.optimizationArray = {}
        global paramIndexes

        for k,v in OrderedDict(self.content.items('main')).items():            
        
            #Add the params names avoiding the optimization line
            if k.find(",") == -1:
                #Add the name of the param and the value
                if k not in self.mainParams:
                    self.mainParams[k] = {}
                self.mainParams[k]['value'] = v
                #Check for optimization flags
                if self.content.has_option('main', k+',F'):
                    if self.content.getboolean('main', k+',F') == 1:
                        self.mainParams[k]['optimize'] = True
                        self.mainParams[k]['start'] = self.content.getfloat('main', k+',1')
                        self.mainParams[k]['step'] = self.content.getfloat('main', k+',2')
                        self.mainParams[k]['stop'] = self.content.getfloat('main', k+',3')
                        self.optimizationArray[paramIndexes[k]] = (self.mainParams[k]['start'],  self.mainParams[k]['step'],  self.mainParams[k]['stop'])
                    else: self.mainParams[k]['optimize'] = False
                else:
                    self.mainParams[k]['optimize'] = False
                
        i = 0

        
        for k,v in list(OrderedDict(self.content.items('additional')).items()):
        
            #Add the params names avoiding the optimization line
            if k.find(",") == -1:
                #Add the name of the param and the value
                if k not in self.additionalParams:
                    self.additionalParams[k] = {}
                self.additionalParams[k]['value'] = v
                #Check for optimization flags
                if self.content.has_option('additional', k+',F'):
                    if self.content.getint('additional', k+',F') == 1: 
                        self.additionalParams[k]['optimize'] = True
                        self.additionalParams[k]['start'] = self.content.getfloat('additional', k+',1')
                        self.additionalParams[k]['step'] = self.content.getfloat('additional', k+',2')
                        self.additionalParams[k]['stop'] = self.content.getfloat('additional', k+',3')
                        self.optimizationArray[paramIndexes[k]] = (self.additionalParams[k]['start'],  self.additionalParams[k]['step'],  self.additionalParams[k]['stop'])     
                    else: self.additionalParams[k]['optimize'] = False
                else:
                    self.additionalParams[k]['optimize'] = False
                i += 1
