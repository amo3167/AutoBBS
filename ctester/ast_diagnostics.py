#!/usr/bin/env python3
import subprocess
import sys
import linecache
import os
from os import remove

def main():

    print("Searching and deleting previous diagnostics...")
    
    for file in os.listdir(os.path.join(os.getcwd())):
        if "diagnostic" in file :
            if "diagnostics" not in file:
                remove(file)

    print("All old files deleted. Now starting new diagnostic runs...")
    
    #first diagnostic
    print("Running diagnostic test 1 - (teyacanani standard test).")
    subprocess.check_output("python3 asirikuy_strategy_tester.py -ot diagnostic1 -c ./config/diagnostics/ast_diagnostic1.config", shell=True)
   
    #second diagnostic
    print("Running diagnostic test 2 - (sapaq usd/jpy standard test).")
    subprocess.check_output("python3 asirikuy_strategy_tester.py -ot diagnostic2 -c ./config/diagnostics/ast_diagnostic2.config", shell=True)
   
    #third diagnostic
    print("Running diagnostic test 3 - (quimichi multi-pair eur/usd & gbp/usd & usd/jpy standard test).")
    subprocess.check_output("python3 asirikuy_strategy_tester.py -ot diagnostic3 -c ./config/diagnostics/ast_diagnostic3.config", shell=True)
   
    #fourth diagnostic
    print("Running diagnostic test 4 - (sapaq usd/jpy + watukushay fe BB eur/usd + gg ATR gbp/usd portfolio standard test).")
    subprocess.check_output("python3 asirikuy_strategy_tester.py -ot diagnostic4 -c ./config/diagnostics/ast_diagnostic4.config", shell=True)
   
    #fifth diagnostic
    print("Running diagnostic test 5 - (watukushay fe bb optimization, brute force test (single core)).")
    subprocess.check_output("python3 asirikuy_strategy_tester.py -oo diagnostic5 -c ./config/diagnostics/ast_diagnostic5.config", shell=True)
   
    #sixth diagnostic
    print("Running diagnostic test 6 - (watukushay fe bb optimization, brute force test (dual core)).")
    subprocess.check_output("python3 asirikuy_strategy_tester.py -oo diagnostic6 -c ./config/diagnostics/ast_diagnostic6.config", shell=True)
   
    #seventh diagnostic
    print("Running diagnostic test 7 - (watukushay fe rsi optimization, genetics test (single core)).")
    subprocess.check_output("python3 asirikuy_strategy_tester.py -oo diagnostic7 -c ./config/diagnostics/ast_diagnostic7.config", shell=True)
   
    #eigth diagnostic
    print("Running diagnostic test 8 - (watukushay fe rsi optimization, genetics test (dual core)).")
    subprocess.check_output("python3 asirikuy_strategy_tester.py -oo diagnostic8 -c ./config/diagnostics/ast_diagnostic8.config", shell=True)
   
    #all diagnostics done
    print("Diagnostic tests finished :o)")
    
            
##################################
###           MAIN           ####
##################################

if __name__ == "__main__": main()
