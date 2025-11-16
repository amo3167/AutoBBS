import sys, argparse, os
sys.path.insert(0, './include')
from asirikuy import *
from graphics import *

parser = argparse.ArgumentParser()
parser.add_argument('-o', '--optimization-file')
parser.add_argument('-t', '--test-file')
parser.add_argument('-p', '--portfolio-file')
parser.add_argument('-g', '--goal')
parser.add_argument('--pdf', action = 'store_true')
parser.add_argument('--show', action = 'store_true')
args = parser.parse_args()

if args.optimization_file == None and args.test_file == None and args.portfolio_file == None:
	parser.print_help()
	quit()

if args.test_file != None:
    fileName, fileExtension = os.path.splitext(args.test_file)
    plotTestResult(args.test_file, pdf = args.pdf, show = args.show)
    
if args.optimization_file != None:
    goal = 0
    fileName, fileExtension = os.path.splitext(args.optimization_file)
    if args.goal != None:
        goal = int(args.goal)
    plotOptimizationResult(args.optimization_file, goal, pdf = args.pdf, show = args.show)

if args.portfolio_file != None:
    fileName, fileExtension = os.path.splitext(args.portfolio_file)
    plotPortfolioTestResult(args.portfolio_file, pdf = args.pdf, show = args.show)

print("File saved to: " + fileName + ".png")
