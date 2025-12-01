
import os
import xml.dom.minidom
from include.platform_utils import is_windows, is_unix

def prettyXML(filePath):
	xmlContent = xml.dom.minidom.parse(filePath) 
	pretty_xml = xmlContent.toprettyxml()
	with open(filePath, "w", encoding='utf-8') as f:
		f.write(pretty_xml)

def insertLine(filePath, line, numberLine):
	with open(filePath, "r", encoding='utf-8') as f:
		contents = f.readlines()
	
	contents.insert(numberLine, line)
	
	with open(filePath, "w", encoding='utf-8') as f:
		f.write("\n".join(contents))

def clearScreen():
	"""Clear the terminal screen (cross-platform)"""
	if is_unix():
		os.system('clear')
	elif is_windows():
		os.system('cls')