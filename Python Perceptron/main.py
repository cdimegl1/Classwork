#!/usr/bin/python3
import sys
import os
import string
import random
from nltk.stem.snowball import SnowballStemmer

def testExample(doc, vocab, weights, freq):
    result = 0
    for word in doc:
        if word in vocab:
            freq[word] = freq.get(word, 0) + 1
    for word in freq.keys():
        result += freq[word] * weights[word]
    result += weights['**bias**']
    return 1 if result > 0 else -1

def trainPerceptron(hamDocs, spamDocs, vocab, iterations, learningRate):
    weights = dict.fromkeys(vocab)
    for word in weights.keys():
        weights[word] = random.uniform(-.1, .1)
    weights['**bias**'] = random.uniform(-.1, .1)
    for i in range(iterations):
        print(i + 1)
        for doc in spamDocs:
            freq = dict.fromkeys(vocab, 0)
            test = testExample(doc, vocab, weights, freq)
            for word in doc:
                weights[word] += learningRate * (-1 - test) * freq[word]
            weights['**bias**'] += learningRate * (-1 - test)
        for doc in hamDocs:
            freq = dict.fromkeys(vocab, 0)
            test = testExample(doc, vocab, weights, freq)
            for word in doc:
                weights[word] += learningRate * (1 - test) * freq[word]
            weights['**bias**'] += learningRate * (1 - test)
    return weights

def main():
    dataFolder = sys.argv[1]
    hamFolder = dataFolder + '/ham'
    spamFolder = dataFolder + '/spam'
    vocab = set()
    hamDocs = []
    spamDocs = []
    stemmer = SnowballStemmer('english')
    stopWords = ['a','about','above','after','again','against','all','am','an','and','any','are','arent','as','at','be','because','been','before','being','below','between','both','but','by','cant','cannot','could','couldnt','did','didnt','do','does','doesnt','doing','dont','down','during','each','few','for','from','further','had','hadnt','has','hasnt','have','havent','having','he','hed','hell','hes','her','here','heres','hers','herself','him','himself','his','how','hows','i','id','ill','im','ive','if','in','into','is','isnt','it','its','its','itself','lets','me','more','most','mustnt','my','myself','no','nor','not','of','off','on','once','only','or','other','ought','our','ours 	','ourselves','out','over','own','same','shant','she','shed','shell','shes','should','shouldnt','so','some','such','than','that','thats','the','their','theirs','them','themselves','then','there','theres','these','they','theyd','theyll','theyre','theyve','this','those','through','to','too','under','until','up','very','was','wasnt','we','wed','well','were','weve','were','werent','what','whats','when','whens','where','wheres','which','while','who','whos','whom','why','whys','with','wont','would','wouldnt','you','youd','youll','youre','youve','your','yours','yourself','yourselves']
    for filename in os.listdir(hamFolder):
        with open(hamFolder + '/' + filename, 'r', encoding = 'latin1') as doc:
            words = doc.read()
            words = words.translate(str.maketrans('', '', string.punctuation)).split()
            if sys.argv[5] == 'yes':
                words = [word for word in words if word not in stopWords]
            hamDocs.append([stemmer.stem(word) for word in words])
    for filename in os.listdir(spamFolder):
        with open(spamFolder + '/' + filename, 'r', encoding = 'latin1') as doc:
            words = doc.read()
            words = words.translate(str.maketrans('', '', string.punctuation)).split()
            if sys.argv[5] == 'yes':
                words = [word for word in words if word not in stopWords]
            spamDocs.append([stemmer.stem(word) for word in words])
    for doc in hamDocs:
        for word in doc:
            vocab.add(word)
    for doc in spamDocs:
        for word in doc:
            vocab.add(word)
    weights = trainPerceptron(hamDocs, spamDocs, vocab, int(sys.argv[3]), float(sys.argv[4]))
    testFolder = sys.argv[2]
    hamFolder = testFolder + '/ham'
    spamFolder = testFolder + '/spam'
    hamDocs = []
    spamDocs = []
    for filename in os.listdir(hamFolder):
        with open(hamFolder + '/' + filename, 'r', encoding = 'latin1') as doc:
            words = doc.read()
            words = words.translate(str.maketrans('', '', string.punctuation)).split()
            if sys.argv[5] == 'yes':
                words = [word for word in words if word not in stopWords]
            hamDocs.append([stemmer.stem(word) for word in words])
    for filename in os.listdir(spamFolder):
        with open(spamFolder + '/' + filename, 'r', encoding = 'latin1') as doc:
            words = doc.read()
            words = words.translate(str.maketrans('', '', string.punctuation)).split()
            if sys.argv[5] == 'yes':
                words = [word for word in words if word not in stopWords]
            spamDocs.append([stemmer.stem(word) for word in words])
    
    numRight = 0
    for doc in hamDocs:
        freq = {}
        if testExample(doc, vocab, weights, freq) == 1:
            numRight += 1
    for doc in spamDocs:
        freq = {}
        if testExample(doc, vocab, weights, freq) == -1:
            numRight += 1
    print(numRight / (len(hamDocs) + len(spamDocs)))

if __name__ == '__main__':
    main()