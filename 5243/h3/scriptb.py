import nltk
nltk.download('punkt')
import string
import random
from collections import Counter
import heapq
import math

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

def split_dataset():
    global training_set
    global validation_set
    global testing_set
    print("Splitting dataset")
    idx_list = list(range(0, 3000))
    random.shuffle(idx_list)
    training_set = idx_list[0:1800]
    validation_set = idx_list[1800:2400]
    testing_set = idx_list[2400:3000]

def build_D2():
    print("Building pruned feature set of size 1000")
    freq_words = heapq.nlargest(1000, word_dict, key=lambda e: word_dict[e][1])
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
        diff = diff + math.pow(a_counter[idx] - b_counter[idx], 2)

    diff = math.sqrt(diff)

    return diff

def run_k_nn(input_data, k):
    global training_set
    nearest_neighbors = heapq.nsmallest(k, training_set, key=lambda e: abs_diff(input_data, D[e]))
    nearest_neighbors_c = [classification_list[i] for i in nearest_neighbors]
    print(nearest_neighbors_c)
    return max(nearest_neighbors_c, key=nearest_neighbors_c.count)

def run_k_validation(fv, k):
    global validation_set
    correct = 0
    total = 0

    for s in validation_set:
        input_data = fv[s]
        sentiment = classification_list[s]
        computed_sentiment = run_k_nn(input_data, k)
        if computed_sentiment == sentiment:
            correct = correct + 1
        total = total + 1
    return correct / total

def run_k_testing(fv, k):
    global testing_set
    correct = 0
    total = 0

    for s in testing_set:
        input_data = fv[s]
        sentiment = classification_list[s]
        computed_sentiment = run_k_nn(input_data, k)
        if computed_sentiment == sentiment:
            correct = correct + 1
        total = total + 1
    return correct / total

def get_best_k(fv):
    best_k = 0
    best_k_score = 0
    for k in range(2):
        print("Testing k-NN on validation set with k: {}".format(k+1))
        k_score = run_k_validation(fv, k+1)
        print("Accuracy: {}".format(k_score))
        if k_score > best_k_score:
            best_k = k
            best_k_score = k_score
        
    print()
    return best_k+1


with open('all.txt') as f:
    for line in f:
        sentence_raw, sentiment_raw = line.split('\t')
        sentence_list.append(sentence_raw)
        D.append(list())
        sentiment = int(sentiment_raw)
        classification_list.append(sentiment)
        sentence = sentence_raw.translate(translator).lower()
        tokens = nltk.word_tokenize(sentence)
        
        for token in tokens:
            if token in word_dict:
                word_entry = word_dict[token]
                D[-1].append(word_entry[0])
                word_entry[1] = word_entry[1] + 1
            else:
                word_dict[token] = [word_count, 1]
                D[-1].append(word_count)
                word_count = word_count + 1
    print("Feature vector construction complete.")
    print("M (Number of sentences): {}".format(len(D)))
    print("N (Number of unique words): {}".format(word_count))
    print()

    split_dataset()
    build_D2()

    ## Run kNN related functions
    k = get_best_k(D)
    print("Running k-NN on the testing set using k: {}".format(k))

    knn_accuracy = run_k_testing(D, k)
    print("Testing set accuracy: {}".format(knn_accuracy))
    print()
    print("Running k-NN on the pruned feature vector")
    k = get_best_k(D2)
    print("Running k-NN on the testing set using k: {}".format(k))

    knn_accuracy = run_k_testing(D2, k)
    print("Testing set accuracy: {}".format(knn_accuracy))