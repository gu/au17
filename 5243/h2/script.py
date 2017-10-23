import nltk
import string

word_list = list()
sentence_list = list()
D = list()

translator = str.maketrans('', '', string.punctuation)

with open('test.txt') as f:
    for line in f:
        sentence_raw, sentiment_raw = line.split('\t')
        sentence_list.append(sentence_raw)
        D.append([0] * len(word_list))
        sentiment = int(sentiment_raw)
        sentence = sentence_raw.translate(translator).lower()
        tokens = nltk.word_tokenize(sentence)
        
        for token in tokens:
            print(token)
            if token in word_list:
                word_idx = word_list.index(token)
                D[-1][word_idx] = D[-1][word_idx] + 1
            else:
                word_list.append(token)
                D[-1].append(1)

