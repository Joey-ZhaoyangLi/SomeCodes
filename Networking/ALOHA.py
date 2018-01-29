import random
import math


class Node:
    def __init__(self, p):
        self.p = p

    def transmit(self):
        # return random.uniform(0, 1.0) <= self.p
        if random.uniform(0, 1.0) <= self.p:
            self.flag = 1
        else:
            self.flag = 0
        return self.flag

def main():
    num_of_nodes = 8
    prob = 0.5
    nodes = []
    for i in range(num_of_nodes):
        node = Node(prob)
        nodes.append(node)
    slots = 100000
    transmit = False
    collision = False
    num_transmission = 0
    for i in range(slots):
        flag = 0
        for node in nodes:
            flag += node.transmit()
        if flag == 1:
            num_transmission += 1

    expected_efficiency = num_of_nodes * prob * math.pow(1-prob, num_of_nodes-1)
    print('theoretical efficiency: ', end=' ')
    print(expected_efficiency)
    print('observed efficiency: ', end=' ')
    efficiency = num_transmission / slots
    print(efficiency)

# As printed, the observed efficiency is consistent with the theoretical efficiency.

if __name__ == '__main__':
    main()