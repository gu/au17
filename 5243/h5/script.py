"""
Problem 6 for Homework 5
Author: Freddy Gu
"""

import string
import time
import nltk
from datasketch import MinHash

nltk.download('punkt')

WORD_DICT = {}
WORD_COUNT = 0
SENTENCE_LIST = list()
CLASSIFICATION_LIST = list()
D = list()
B = list()
M = list()
M_LIST = list()

TRANSLATOR = str.maketrans('', '', string.punctuation)


def build_baseline_matrix():
    """
    Generate len(D) by len(D) matrix with associated Jaccard similarity values.
    """
    print("Building baseline matrix.")

    for r_idx in range(0, len(SENTENCE_LIST)):
        for c_idx in range(0, len(SENTENCE_LIST)):
            # if (r_idx == c_idx):
            #     B[r_idx][c_idx] = 1.0
            # elif (c_idx < r_idx):
            #     B[r_idx][c_idx] = B[c_idx][r_idx]
            # else:
            a_set = set(D[r_idx])
            b_set = set(D[c_idx])
            num = len(a_set.intersection(b_set))
            denom = len(a_set.union(b_set))
            B[r_idx][c_idx] = num/denom

    print("Done building baseline matrix.")
    print()


def run_kminhash(k):
    """
    Generate len(D) by len(D) matrix with estimated Jaccard similarity values.
    """
    print("Running k-minhash method with {} permutations.".format(k))

    for r_idx in range(0, len(SENTENCE_LIST)):
        for c_idx in range(0, len(SENTENCE_LIST)):
            # if c_idx < r_idx:
            #     M[r_idx][c_idx] = M[c_idx][r_idx]
            # else:
            m1 = M_LIST[r_idx]
            m2 = M_LIST[c_idx]

            M[r_idx][c_idx] = m1.jaccard(m2)

def calculateMSE():
    """
    Calculate mean squared error between baseline set B and the kminhash set M
    """
    ret = 0

    for r_idx in range(0, len(SENTENCE_LIST)):
        for c_idx in range(0, len(SENTENCE_LIST)):
            mse = (B[r_idx][c_idx] - M[r_idx][c_idx]) ** 2
            ret = ret + mse
    return ret / (len(SENTENCE_LIST) * len(SENTENCE_LIST))

def build_mlist(k):
    for i in range(0, len(SENTENCE_LIST)):
        m = MinHash(num_perm=k)
        for d in D[i]:
            m.update(str(d).encode('utf8'))
        M_LIST[i] = m

with open("all.txt") as f:
    for line in f:
        sentence_raw, sentiment_raw = line.split("\t")
        sentiment = int(sentiment_raw)

        SENTENCE_LIST.append(sentence_raw)
        D.append(list())
        CLASSIFICATION_LIST.append(sentiment)

        sentence = sentence_raw.translate(TRANSLATOR).lower()
        tokens = nltk.word_tokenize(sentence)

        for token in tokens:
            if token in WORD_DICT:
                word_entry = WORD_DICT[token]
                D[-1].append(word_entry)
            else:
                WORD_DICT[token] = WORD_COUNT
                D[-1].append(WORD_COUNT)
                WORD_COUNT += 1

    print("Feature vector construction complete.")
    print("M (Number of sentences): {}".format(len(D)))
    print("N (Number of unique words): {}".format(WORD_COUNT))
    print()

    B = [[0]*len(SENTENCE_LIST) for x in range(0, len(SENTENCE_LIST))]

    time_start = time.time()
    build_baseline_matrix()
    time_end = time.time()
    print("Time to complete: {} seconds.".format(time_end - time_start))


    M_LIST = [0] * len(SENTENCE_LIST)

    for k in [16, 32, 64, 128, 256]:
        M = [[0]*len(SENTENCE_LIST) for x in range(0, len(SENTENCE_LIST))]
        time_start = time.time()
        build_mlist(k)
        run_kminhash(k)
        time_end = time.time()
        print("Completed kminhash run.")
        error = calculateMSE()
        print("Mean square error for k={} is {}.".format(k, error))
        print("Time to complete: {} seconds.".format(time_end - time_start))
        print()
