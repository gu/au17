import nltk
import string
from collections import Counter
import math
import random

import time

nltk.download('punkt')

word_dict = {}
word_count = 0
sentence_list = list()
classification_list = list()
freq_words_idx = list()
D = list()
D2 = list()

translator = str.maketrans('', '', string.punctuation)

training_set = list()
validation_set = list()
testing_set = list()


def build_d2():
    global freq_words_idx
    print("Building pruned feature set of size 1000")
    freq_words = sorted(word_dict, key=lambda e: word_dict[e][1])[word_count - 1000:]
    freq_words_idx = [word_dict[freq_word][0] for freq_word in freq_words]

    for row in D:
        D2.append([i for i in row if i in freq_words_idx])

    print("Pruned feature vector construction complete.")
    print("M (Number of sentences): {}".format(len(D2)))
    print("N (Number of unique words): {}".format(len(freq_words_idx)))
    print()


def abs_diff(a, b):
    a_counter = Counter(a)
    b_counter = Counter(b)
    diff = 0

    all_idx = list(set(a_counter) | set(b_counter))

    for idx in all_idx:
        diff += math.pow(a_counter[idx] - b_counter[idx], 2)

    diff = math.sqrt(diff)

    return diff


def run_k_nn(input_data, k, fv):
    global training_set
    nearest_neighbors = sorted(training_set, key=lambda e: abs_diff(input_data, fv[e]))[:k]
    nearest_neighbors_c = [classification_list[i] for i in nearest_neighbors]
    return max(nearest_neighbors_c, key=nearest_neighbors_c.count)


def run_k_validation(fv, k):
    global validation_set
    correct = 0
    total = 0

    for s in validation_set:
        input_data = fv[s]
        time1 = time.time()
        computed_sentiment = run_k_nn(input_data, k, fv)
        time2 = time.time()
        print("time: {}".format(time2-time1))
        correct += 1 if computed_sentiment == classification_list[s] else 0
        total += 1
    return correct / total


def run_k_testing(fv, k):
    global testing_set
    correct = 0
    total = 0

    for s in testing_set:
        input_data = fv[s]
        
        computed_sentiment = run_k_nn(input_data, k, fv)
        correct += 1 if computed_sentiment == classification_list[s] else 0
        total += 1
    return correct / total


def get_best_k(fv):
    best_k = 0
    best_k_score = 0
    for k in range(10):
        print("Testing k-NN on validation set with k: {}".format(k + 1))
        k_score = run_k_validation(fv, k + 1)
        print("Accuracy: {}".format(k_score))
        if k_score > best_k_score:
            best_k = k
            best_k_score = k_score

    print()
    return best_k + 1


def run_n_bayes(pruned, fv):
    correct = 0
    total = 0
    training_set_bayes = training_set + validation_set

    c1 = [x for x in training_set_bayes if classification_list[x] == 1]
    c1_dict = {}
    for s_idx in c1:
        data_set_tmp = set(fv[s_idx])
        for w_idx in data_set_tmp:
            if w_idx in c1_dict:
                c1_dict[w_idx] += 1
            else:
                c1_dict[w_idx] = 1

    c2 = [x for x in training_set_bayes if classification_list[x] == 0]
    c2_dict = {}
    for s_idx in c2:
        data_set_tmp = set(fv[s_idx])
        for w_idx in data_set_tmp:
            if w_idx in c2_dict:
                c2_dict[w_idx] += 1
            else:
                c2_dict[w_idx] = 1

    training_set_bayes_l = len(training_set_bayes)
    p_c1 = len(c1) / training_set_bayes_l
    p_c2 = len(c2) / training_set_bayes_l

    range_to_check = freq_words_idx if pruned else range(word_count)

    time1 = time.time()
    for x_idx in testing_set:
        x = fv[x_idx]
        p_s_c1 = 1
        p_s_c2 = 1
        for word_idx in range_to_check:
            c1_f = (c1_dict[word_idx] if word_idx in c1_dict else 0)
            c2_f = (c2_dict[word_idx] if word_idx in c2_dict else 0)
            if word_idx in x:
                p_x_c1 = c1_f / len(c1)
                p_s_c1 *= p_x_c1
                p_x_c2 = c2_f / len(c1)
                p_s_c2 *= p_x_c2
            else:
                p_x_c1 = (len(c1) - c1_f) / len(c1)
                p_s_c1 *= p_x_c1
                p_x_c2 = (len(c2) - c2_f) / len(c2)
                p_s_c2 *= p_x_c2

        computed_sentiment = 1 if (p_s_c1*p_c1) > (p_s_c2*p_c2) else 0
        correct += 1 if computed_sentiment == classification_list[x_idx] else 0
        total += 1

    time2 = time.time()
    print("time: {}".format(time2-time1))
    return correct/total

with open("all.txt") as f:
    for line in f:
        sentence_raw, sentiment_raw = line.split("\t")
        sentiment = int(sentiment_raw)

        sentence_list.append(sentence_raw)
        D.append(list())
        classification_list.append(sentiment)

        sentence = sentence_raw.translate(translator).lower()
        tokens = nltk.word_tokenize(sentence)

        r = random.random()
        if r < 0.8:
            training_set.append(len(D) - 1)
        elif 0.8 <= r < 0.9:
            validation_set.append(len(D) - 1)
        elif r >= 0.9:
            testing_set.append(len(D) - 1)

        for token in tokens:
            if token in word_dict:
                word_entry = word_dict[token]
                D[-1].append(word_entry[0])
                word_entry[1] += 1
            else:
                word_dict[token] = [word_count, 1]
                D[-1].append(word_count)
                word_count += 1

    print("Feature vector construction complete.")
    print("M (Number of sentences): {}".format(len(D)))
    print("N (Number of unique words): {}".format(word_count))
    print()

    build_d2()

    # Run kNN related functions
    # k = get_best_k(D)
    # print("Running k-NN on the testing set using k: {}".format(k))

    # knn_accuracy = run_k_testing(D, k)
    # print("Testing set accuracy: {}".format(knn_accuracy))
    # print()
    # print("Running k-NN on the pruned feature vector")
    # k = get_best_k(D2)
    # print("Running k-NN on the testing set using k: {}".format(k))

    # knn_accuracy = run_k_testing(D2, k)
    # print("Testing set accuracy: {}".format(knn_accuracy))
    # print()

    # Run Naive Bayes Classifier
    # print("Running Naive Bayes Classifier on original dataset.")
    # nb_accuracy = run_n_bayes(False, D)
    # print("Testing set accuracy: {}".format(nb_accuracy))

    print("Running Naive Bayes Classifier on pruned dataset.")
    nb_accuracy = run_n_bayes(True, D2)
    print("Testing set accuracy: {}".format(nb_accuracy))