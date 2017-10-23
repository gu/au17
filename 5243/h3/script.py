import nltk
nltk.download('punkt')
import string
import random
import numpy

word_list = list()
sentence_list = list()
D = list()
D2 = list()

translator = str.maketrans('', '', string.punctuation)

training_set = list()
validation_set = list()
testing_set = list()

def split_dataset():
    print("Splitting dataset")
    idx_list = list(range(0, 3000))
    random.shuffle(idx_list)
    training_set = idx_list[0:1800]
    validation_set = idx_list[1800:2400]
    testing_set = idx_list[2400:3000]

def sum_column(col):
    total = 0
    for row in range(len(D)):
        if (col < len(D[row])):
            total += D[row][col]
        else:
            continue
    return total

def build_D2():
    print("Building pruned feature set of size 1000")
    column_sum = list()
    for word_idx in range(len(word_list)):
        column_sum.append(sum_column(word_idx))

    important_idx_set = sorted(sorted(range(len(column_sum)), key=lambda i: column_sum[i])[-1000:])
    for row in D:
        new_row = [row[i] for i in important_idx_set if i < len(row)]
        D2.append(new_row)

    print("Pruned feature vector construction complete.")
    print("M (Number of sentences): {}".format(len(D2)))
    print("N (Number of unique words): {}".format(len(D2[-1])))

def abs_diff(a, b):
    ret = 0
    len_a = len(a)
    len_b = len(b)


    return ret


with open('all.txt') as f:
    for line in f:
        sentence_raw, sentiment_raw = line.split('\t')
        sentence_list.append(sentence_raw)
        D.append([0] * len(word_list))
        sentiment = int(sentiment_raw)
        sentence = sentence_raw.translate(translator).lower()
        tokens = nltk.word_tokenize(sentence)
        
        for token in tokens:
            if token in word_list:
                word_idx = word_list.index(token)
                D[-1][word_idx] = D[-1][word_idx] + 1
            else:
                word_list.append(token)
                D[-1].append(1)
    print("Feature vector construction complete.")
    print("M (Number of sentences): {}".format(len(D)))
    print("N (Number of unique words): {}".format(len(D[-1])))
    print()

    split_dataset()
    build_D2()