#!/usr/bin/python3
import sys
import os
import string
import math
from nltk.stem.snowball import SnowballStemmer

def main():
    dataFolder = sys.argv[1]
    hamFolder = dataFolder + '/ham'
    spamFolder = dataFolder + '/spam'
    vocab = set()
    hamDocs = []
    hamFreq = {}
    spamDocs = []
    spamFreq = {}
    hamCondProb = {}
    spamCondProb = {}
    stemmer = SnowballStemmer('english')
    stopWords = ['a','about','above','after','again','against','all','am','an','and','any','are','arent','as','at','be','because','been','before','being','below','between','both','but','by','cant','cannot','could','couldnt','did','didnt','do','does','doesnt','doing','dont','down','during','each','few','for','from','further','had','hadnt','has','hasnt','have','havent','having','he','hed','hell','hes','her','here','heres','hers','herself','him','himself','his','how','hows','i','id','ill','im','ive','if','in','into','is','isnt','it','its','its','itself','lets','me','more','most','mustnt','my','myself','no','nor','not','of','off','on','once','only','or','other','ought','our','ours 	','ourselves','out','over','own','same','shant','she','shed','shell','shes','should','shouldnt','so','some','such','than','that','thats','the','their','theirs','them','themselves','then','there','theres','these','they','theyd','theyll','theyre','theyve','this','those','through','to','too','under','until','up','very','was','wasnt','we','wed','well','were','weve','were','werent','what','whats','when','whens','where','wheres','which','while','who','whos','whom','why','whys','with','wont','would','wouldnt','you','youd','youll','youre','youve','your','yours','yourself','yourselves']
    for filename in os.listdir(hamFolder):
        with open(hamFolder + '/' + filename, 'r', encoding = 'latin1') as doc:
            words = doc.read()
            words = words.translate(str.maketrans('', '', string.punctuation)).split()
            if sys.argv[3] == 'yes':
                words = [word for word in words if word not in stopWords]
            hamDocs.append([stemmer.stem(word) for word in words])
    for filename in os.listdir(spamFolder):
        with open(spamFolder + '/' + filename, 'r', encoding = 'latin1') as doc:
            words = doc.read()
            words = words.translate(str.maketrans('', '', string.punctuation)).split()
            if sys.argv[3] == 'yes':
                words = [word for word in words if word not in stopWords]
            spamDocs.append([stemmer.stem(word) for word in words])
    docs = hamDocs + spamDocs
    for doc in hamDocs:
        for word in doc:
            vocab.add(word)
            hamFreq[word] = hamFreq.get(word, 0) + 1
    for doc in spamDocs:
        for word in doc:
            vocab.add(word)
            spamFreq[word] = spamFreq.get(word, 0) + 1
    numDocs = len(docs)
    priorHam = math.log(numDocs / len(hamDocs))
    priorSpam = math.log(numDocs / len(spamDocs))
    for v in vocab:
        hamCondProb[v] = math.log((hamFreq.get(v, 0) + 1) / (sum(hamFreq.values()) + len(vocab)))
        spamCondProb[v] = math.log((spamFreq.get(v, 0) + 1) / (sum(spamFreq.values()) + len(vocab)))
    testFolder = sys.argv[2]
    hamFolder = testFolder + '/ham'
    spamFolder = testFolder + '/spam'
    hamDocs = []
    spamDocs = []
    for filename in os.listdir(hamFolder):
        with open(hamFolder + '/' + filename, 'r', encoding = 'latin1') as doc:
            words = doc.read()
            words = words.translate(str.maketrans('', '', string.punctuation)).split()
            if sys.argv[3] == 'yes':
                words = [word for word in words if word not in stopWords]
            hamDocs.append([stemmer.stem(word) for word in words])
    for filename in os.listdir(spamFolder):
        with open(spamFolder + '/' + filename, 'r', encoding = 'latin1') as doc:
            words = doc.read()
            words = words.translate(str.maketrans('', '', string.punctuation)).split()
            if sys.argv[3] == 'yes':
                words = [word for word in words if word not in stopWords]
            spamDocs.append([stemmer.stem(word) for word in words])
    extractedVocab = []
    numRight = 0
    for doc in hamDocs:
        for word in doc:
            if word in vocab:
                extractedVocab.append(word)
        hamScore = priorHam
        spamScore = priorSpam
        for word in extractedVocab:
            hamScore += hamCondProb[word]
            spamScore += spamCondProb[word]
        if hamScore > spamScore:
            numRight += 1
        extractedVocab = []
    for doc in spamDocs:
        for word in doc:
            if word in vocab:
                extractedVocab.append(word)
        hamScore = priorHam
        spamScore = priorSpam
        for word in extractedVocab:
            hamScore += hamCondProb[word]
            spamScore += spamCondProb[word]
        if spamScore > hamScore:
            numRight += 1
        extractedVocab = []
    print(numRight / (len(hamDocs) + len(spamDocs)))
    
if __name__ == '__main__':
    main()