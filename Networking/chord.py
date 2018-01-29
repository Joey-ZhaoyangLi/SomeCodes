import copy

'''
Implementation of Chord DHT
'''

input_filename = input("Please enter input file: ")
output_filename = "output.out"

with open(input_filename, "r") as input_file:
    input_strs = input_file.read()

input_strs = input_strs.split('\n')
hash_space = int(input_strs[0])
num_nodes = int(input_strs[1])
num_keys = int(input_strs[2])
node_ids = input_strs[3].split(',')
key_ids = input_strs[4].split(',')
node_ids = [int(nodeid) % (2 ** hash_space) for nodeid in node_ids]
key_ids = [int(keyid) % (2 ** hash_space) for keyid in key_ids]

query = []
for line in input_strs[5:]:
    if line == '-1,-1':
        break
    key, id = line.split(',')
    query.append((int(key) % (2 ** hash_space), int(id) % (2 ** hash_space)))

nodes = []
nodeid2index = {}


class Node:
    def __init__(self, id):
        self.__id = id
        self.__key = []
        self.__succTable = [[None for i in range(3)] for i in range(hash_space)]
        for i in range(hash_space):
            self.__succTable[i][0] = i
            self.__succTable[i][1] = (id + 2 ** i) % (2 ** hash_space)

    def update(self, node_ids):
        for i in range(hash_space):
            for j in range(self.__succTable[i][1], self.__succTable[i][1] + 2 ** hash_space):
                if j % (2 ** hash_space) in node_ids:
                    self.__succTable[i][2] = j % (2 ** hash_space)
                    break

    def __str__(self):
        string = ''
        for i in range(hash_space):
            for j in range(3):
                string += str(self.__succTable[i][j])
                string += ' '
            string += '\n'

        return string

    def insert_key(self, key):
        self.__key.append(key)

    def search_key(self, key, path):
        path += str(self.__id) + ' '
        if key in self.__key:
            return path
        else:
            next = 0

            if key < self.__id:
                index = 0
                for i in range(hash_space):
                    if self.__succTable[i][2] < self.__id:
                        if self.__succTable[i][2] > key:
                            index = i - 1
                        else:
                            index = i
                        break
                    if i == hash_space - 1:
                        next = self.__succTable[i][2]
                        index = hash_space

                for i in range(index, hash_space):
                    if self.__succTable[i][2] > key:
                        next = self.__succTable[i - 1][2]
                        break
                    if i == hash_space - 1:
                        next = self.__succTable[i][2]

            else:
                for i in range(hash_space):
                    if self.__succTable[i][2] > key:
                        next = self.__succTable[i - 1][2]
                        break
                    if self.__succTable[i][2] < self.__id:
                        if i == 0:
                            next = self.__succTable[i][2]
                        else:
                            next = self.__succTable[i - 1][2]
                        break
                    if i == hash_space - 1:
                        next = self.__succTable[i][2]
            return nodes[nodeid2index[next]].search_key(key, path)


for nodeid in node_ids:
    node = Node(nodeid)
    nodes.append(node)
    nodeid2index[nodeid] = nodes.index(node)
    for nd in nodes:
        nd.update(node_ids)

for key in key_ids:
    for i in range(key, key + 2 ** hash_space):
        if i % (2 ** hash_space) in node_ids:
            nodes[nodeid2index[i % (2 ** hash_space)]].insert_key(key)
            break

with open(output_filename, "w") as output_file:
    output_file.write(nodes[num_nodes - 1].__str__())
    for key, nodeid in query:
        path = ''
        path = nodes[nodeid2index[nodeid]].search_key(key, path)
        path += '\n'
        output_file.write(path)
