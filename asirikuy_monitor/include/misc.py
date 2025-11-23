
import os, xml.dom.minidom

def prettyXML(filePath):
	xmlContent = xml.dom.minidom.parse(filePath) 
	pretty_xml = xmlContent.toprettyxml()
	f = open(filePath, "w")
	f.write(pretty_xml)
	f.close()

def insertLine(filePath, line, numberLine):
	f = open(filePath, "r")
	contents = f.readlines()
	f.close()
	
	contents.insert(numberLine, line)
	
	f = open(filePath, "w")
	contents = "\n".join(contents)
	f.write(contents)
	f.close()

def clearScreen():
	if os.name == 'posix':
		os.system('clear')
	elif os.name == 'nt':
		os.system('cls')