import math

def countf(data, feature):
    att_class = 0
    notatt_class = 0
    att_notclass = 0
    notatt_notclass = 0
    for row in data:
        if row[feature] == '1':
            if row['Class'] == '1':
                att_class += 1
            else:
                att_notclass += 1
        elif row['Class'] == '1':
            notatt_class += 1
        else:
            notatt_notclass += 1
    return (att_class, notatt_class, att_notclass, notatt_notclass)
    
def varianceImpurity(data, feature):
    count = countf(data, 'Class')
    classImpurity = count[0] * count[3] / (count[0] + count[3])**2
    count = countf(data, feature)
    attributeImpurity = (count[2] * count[0] / (count[2] + count[0])**2) if count[2] > 0 and count[0] > 0 else 0
    notAttributeImpurity = (count[3] * count[1] / (count[3] + count[1])**2) if count[3] > 0 and count[1] > 0 else 0
    retVal = classImpurity - ((count[1] + count[3]) / sum(list(count)) * notAttributeImpurity + (count[0] + count[2]) / sum(list(count)) * attributeImpurity)
    return retVal
    
def infoGain(data, feature):
    count = countf(data, 'Class')
    classEntropy = -(count[0] / (count[3] + count[0])) * math.log2(count[0] / (count[3] + count[0])) - (count[3] / (count[0] + count[3])) * math.log2(count[3] / (count[3] + count[0])) if count[0] > 0 and count[3] > 0 else 0
    count = countf(data, feature)
    attributeEntropy = -(count[0] / (count[2] + count[0])) * math.log2(count[0] / (count[2] + count[0])) - (count[2] / (count[2] + count[0])) * math.log2(count[2] / (count[2] + count[0])) if count[0] > 0 and count[2] > 0 else 0
    notAttributeEntropy = -(count[1] / (count[3] + count[1])) * math.log2(count[1] / (count[3] + count[1])) - (count[3] / (count[3] + count[1])) * math.log2(count[3] / (count[3] + count[1])) if count[1] > 0 and count[3] > 0 else 0
    retVal = classEntropy - ((count[0] + count[2]) / sum(list(count)) * attributeEntropy + (count[1] + count[3]) / sum(list(count)) * notAttributeEntropy)
    return retVal

def bestAttribute(data, heuristic):
    mostGain = -1
    bestAttribute = ''
    for column in data[0]:
        if column != 'Class':
            gain = infoGain(data, column) if heuristic == '1' else varianceImpurity(data, column)
            if gain > mostGain:
                bestAttribute = column
                mostGain = gain
    return (bestAttribute, mostGain)

class dTree:
    def __init__(self, key):
        self.key = key
        self.left = None
        self.right = None
        
def buildTree(data, root, heuristic):
        if root.key is None:
            split = bestAttribute(data, heuristic)
            attSplit = split[0]
            if(split[1] != 0):
                root.key = attSplit
            else:
                classVal = data[0]['Class']
                root.key = classVal
                return root
                
        if root.left == None:
            leftData = []
            for row in data:
                if len(row) > 1 and row[root.key] == '0':
                    leftData.append(row.copy())
                    del leftData[-1][root.key]
            if leftData:
                root.left = dTree(None)
                root.left = buildTree(leftData, root.left, heuristic)
        if root.right == None:
            rightData = []
            for row in data:
                if len(row) > 1 and row[root.key] == '1':
                    rightData.append(row.copy())
                    del rightData[-1][root.key]
            if rightData:
                root.right = dTree(None)
                root.right = buildTree(rightData, root.right, heuristic)
        return root
        
def treeTraverse(root, row):
    if root.key == '0':
        return '0'
    if root.key == '1':
        return '1'
    if row[root.key] == '0':
        result = treeTraverse(root.left, row)
    if row[root.key] == '1':
        result = treeTraverse(root.right, row)
    return result
    
def prettyPrint(root, indent):
    if root:
        if root.left.key == '0' or root.left.key == '1':
            print('| ' * indent + root.key + ' = 0' + ' : ' + root.left.key)
        else:
            print('| ' * indent + root.key + ' = 0 :')
            prettyPrint(root.left, indent + 1)
        if root.right.key == '0' or root.right.key == '1':
            print('| ' * indent + root.key + ' = 1' + ' : ' + root.right.key)
        else:
            print('| ' * indent + root.key + ' = 1 :')
            prettyPrint(root.right, indent + 1)