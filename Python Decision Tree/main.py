#!/usr/bin/python3
import sys
import csv
import copy
import dTree
        
def readCSV(fileName):
    rows = []
    with open(fileName, 'r') as trainingCSV:
        csvReader = csv.DictReader(trainingCSV)
        for row in csvReader:
            rows.append(row)
    return rows
    
def accuracy(tree, data):
    numRight = 0
    numWrong = 0
    for row in data:
        if dTree.treeTraverse(tree, row) == row['Class']:
            numRight += 1
        else:
            numWrong += 1
    return numRight / (numRight + numWrong)
    
def main():
    training = readCSV(sys.argv[1])
    validation = readCSV(sys.argv[2])
    testing = readCSV(sys.argv[3])
    heuristic = sys.argv[5]
    tree = dTree.dTree(None)
    tree = dTree.buildTree(copy.deepcopy(training), tree, heuristic)
    print('Accuracy on Test Set for Heuristic {}: {:.4f}'.format(heuristic, accuracy(tree, testing)))
    if sys.argv[4] == 'yes':
        dTree.prettyPrint(tree, 0)
            
if __name__ == '__main__':
    main()