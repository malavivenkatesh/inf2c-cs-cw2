import math
import random
import os

HITS = 0
MISSES = 0


class LRU:
    def __init__(self, associativity, n_blocks, block_size):
        self.index_length = int(math.log(n_blocks/associativity, 2))
        self.offset_length = int(math.log(block_size, 2))
        self.tag_length = 32 - self.index_length - self.offset_length
        self.number_of_sets = int((n_blocks / associativity))
        self.number_of_blocks_per_set = associativity
        self.cache = {}
        self.params = (associativity, n_blocks, block_size)

        for i in range(self.number_of_sets):
            self.cache[i] = []

    def get(self, address):
        global HITS, MISSES
        current_tag = (address) >> (self.index_length + self.offset_length)
        current_index = (address >> self.offset_length) & (
            int(math.pow(2, self.index_length)) - 1)
        if current_tag in self.cache[current_index]:
            self.cache[current_index].insert(0, self.cache[current_index].pop(
                self.cache[current_index].index(current_tag)))
            HITS += 1
        elif len(self.cache[current_index]) < self.number_of_blocks_per_set:
            MISSES += 1
            self.cache[current_index].insert(0, current_tag)
        else:
            MISSES += 1
            self.cache[current_index] = self.cache[current_index][:-1]
            self.cache[current_index].insert(0, current_tag)


class FIFO:
    def __init__(self, associativity, n_blocks, block_size):
        self.index_length = int(math.log(n_blocks/associativity, 2))
        self.offset_length = int(math.log(block_size, 2))
        self.tag_length = 32 - self.index_length - self.offset_length
        self.number_of_sets = int((n_blocks / associativity))
        self.number_of_blocks_per_set = associativity
        self.cache = {}
        self.params = (associativity, n_blocks, block_size)

        for i in range(self.number_of_sets):
            self.cache[i] = []

    def get(self, address):
        global HITS, MISSES
        current_tag = (address) >> (self.index_length + self.offset_length)
        current_index = (address >> self.offset_length) & (
            int(math.pow(2, self.index_length)) - 1)
        if current_tag in self.cache[current_index]:

            HITS += 1
        elif len(self.cache[current_index]) < self.number_of_blocks_per_set:
            MISSES += 1
            self.cache[current_index].insert(0, current_tag)
        else:
            MISSES += 1
            self.cache[current_index] = self.cache[current_index][:-1]
            self.cache[current_index].insert(0, current_tag)


def genLRU():
    block_size = random.choice([32, 64])
    n_blocks = random.choice([16, 64, 256, 1024])
    assoc = 2 ** random.randint(0, math.log(n_blocks, 2))
    return LRU(assoc, n_blocks, block_size)


def genFIFO():
    block_size = random.choice([32, 64])
    n_blocks = random.choice([16, 64, 256, 1024])
    assoc = 2 ** random.randint(0, math.log(n_blocks, 2))
    return FIFO(assoc, n_blocks, block_size)


def runLRU(lru):

    with open("mem_trace.txt") as f:
        address = f.readline()
        while address:
            lru.get(int(address[:-1], 16))
            address = f.readline()
        return (HITS, MISSES, round((float(HITS)/(MISSES+HITS))*100, 1))


def runFIFO(fifo):

    with open("mem_trace.txt") as f:
        address = f.readline()
        while address:
            fifo.get(int(address[:-1], 16))
            address = f.readline()
        return (HITS, MISSES, round((float(HITS)/(MISSES+HITS))*100, 1))


for i in range(100):
    HITS = 0
    MISSES = 0
    lru = genLRU()
    fifo = genFIFO()
    res = runLRU(lru)
    HITS = 0
    MISSES = 0
    res_fifo = runFIFO(fifo)
    com = "./mem_sim LRU " + \
        str(lru.params[0])+" "+str(lru.params[1]) + \
        " "+str(lru.params[2])+" mem_trace.txt"
    com_fifo = "./mem_sim FIFO " + \
        str(fifo.params[0])+" "+str(fifo.params[1]) + \
        " "+str(fifo.params[2])+" mem_trace.txt"
    stream = os.popen(com, "r")
    stream_fifo = os.popen(com_fifo, "r")

    l = stream.readline()
    lines = []
    while l:
        lines.append(l[:-1])
        l = stream.readline()
    hits = int(lines[-3].split(":")[-1])
    misses = int(lines[-2].split(":")[-1])
    rate = float(lines[-1].split(":")[-1][:-1])
    print str(lru.params)
    assert res == (hits, misses, rate)

    print "Passed LRU " + str(lru.params)

    lf = stream_fifo.readline()
    lines_fifo = []
    while lf:
        lines_fifo.append(lf[:-1])
        lf = stream_fifo.readline()
    hitsf = int(lines_fifo[-3].split(":")[-1])
    missesf = int(lines_fifo[-2].split(":")[-1])
    ratef = float(lines_fifo[-1].split(":")[-1][:-1])
    assert res_fifo == (hitsf, missesf, ratef)
    print "Passed FIFO" + str(fifo.params)