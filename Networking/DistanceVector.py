import copy
import math


class Node:
    def __init__(self, id, num_nodes):
        self.id = id
        self.num_nodes = num_nodes
        self.neighbors = []
        self.table = [[math.inf for i in range(num_nodes)] for i in range(num_nodes) ]
        self.table[id][id] = 0
        self.prev_table = []

    def add_link(self, other, dis):
        self.table[self.id][other.get_id()] = dis
        self.neighbors.append(other)

    def get_id(self):
        return self.id

    def set_prev_table(self):
        self.prev_table = copy.deepcopy(self.table)

    def update_neighbor(self):
        for neighbor in self.neighbors:
            self.table[neighbor.get_id()] = [dis for dis in neighbor.get_row(neighbor.get_id())]

    def update_dis(self):
        for i in range(self.num_nodes):
            vector = []
            for neighbor in self.neighbors:
                vector.append(self.table[self.id][neighbor.get_id()] +
                              neighbor.get_dis(neighbor.get_id(), i))
            vector.append(self.table[self.id][i])
            self.table[self.id][i] = min(vector)

    def get_dis(self, row, col):
        return self.table[row][col]

    def get_row(self, row):
        return self.table[row]

    def changed(self):
        for i in range(self.num_nodes):
            for j in range(self.num_nodes):
                if self.prev_table[i][j] != self.table[i][j]:
                    return True
        return False

    def __str__(self):
        s = ''
        for i in range(self.num_nodes):
            for j in range(self.num_nodes):
                s += str(self.table[i][j])
                s += '\t'
            s += '\n'
        return s

def main():
    input_filename = 'DVinput.txt'
    with open(input_filename, 'r') as input_file:
        input_strs = input_file.read()
    # interpret
    input_lines = input_strs.split('\n')
    line1 = input_lines[0].split(' ')
    num_nodes = int(line1[0])
    num_links = int(line1[1])
    node_names = input_lines[1].split(' ')
    node_index = {}
    nodes = []
    # initiallize nodes
    for i in range(num_nodes):
        node_index[node_names[i]] = i
        node = Node(i, num_nodes)
        nodes.append(node)
    # add links
    for i in range(num_links):
        token = input_lines[i+2].split(' ')
        nodes[node_index[token[0]]].add_link(nodes[node_index[token[1]]], float(token[2]))
        nodes[node_index[token[1]]].add_link(nodes[node_index[token[0]]], float(token[2]))
    # print the initial tables
    for node in nodes:
        print(node)

    max_round = 1000
    for round in range(1, max_round):
        changed_flag = 0
        # first update neighors of all nodes
        for node in nodes:
            # before update, save the current table to see whether nothing's changed
            node.set_prev_table()
            node.update_neighbor()
        # then update distance to other nodes
        for node in nodes:
            node.update_dis()
            if node.changed():
                changed_flag = 1
        # if nothing is changed in this round, break
        if changed_flag == 0:
            print(round - 1)
            break
        else:
            print('##########')
            for node in nodes:
                print(node)

if __name__ == '__main__':
    main()