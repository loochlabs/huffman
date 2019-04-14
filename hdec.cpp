#include <vector>
#include <string.h>
#include <fstream>
#include <iostream>
#include <bitset>

using namespace std;

class Node
{
public:
	int freq;
	char byte_code;
	Node * left_child;
	Node * right_child;

	Node(int f, char byte = '\0', Node* left = NULL, Node* right = NULL) :
		freq(f), byte_code(byte), left_child(left), right_child(right) {}

	~Node()
	{
		delete left_child;
		delete right_child;
	}
};


//Priorty Heap
class PriorityHeap
{
public:
	vector<Node*> heap;

	PriorityHeap() {}
	~PriorityHeap() {}

	void insert(Node* element) 
	{
		heap.push_back(element);
		heapify_up(heap.size() - 1);
	}

	Node* extract_min() 
	{
		Node* min = heap.front();
		heap[0] = heap.at(heap.size() - 1);
		heap.pop_back();
		heapify_down(0);
		return min;
	}

	void heapify_up(int idx) 
	{
		while (parent(idx) >= 0 && idx > 0 && heap[parent(idx)]->freq > heap[idx]->freq) 
		{
			Node* temp = heap[parent(idx)];
			heap[parent(idx)] = heap[idx];
			heap[idx] = temp;
			idx = parent(idx);
		}
	}

	void heapify_down(int idx) 
	{
		int child = left(idx);
		if (child > 0 && right(idx) > 0 && heap[child]->freq > heap[right(idx)]->freq) 
		{
			child = right(idx);
		}
		if (child > 0) 
		{
			Node* temp = heap[idx];
			heap[idx] = heap[child];
			heap[child] = temp;
			heapify_down(child);
		}
	}

	//parent index
	int parent(int child) 
	{
		if (child != 0) 
		{
			int idx = (child - 1) / 2;
			return idx;
		}
		return -1;
	}

	//left child index
	int left(int parent) 
	{
		int idx = (parent * 2) + 1;
		return idx < heap.size() ? idx : -1;
	}

	//right child index
	int right(int parent) 
	{
		int idx = (parent * 2) + 2;
		return idx < heap.size() ? idx : -1;
	}
};


Node* rebuild_tree(vector<unsigned char>& table, const int end, int& i) 
{
	//check for trailing 0's followed by ending index
	if ((i / 8) >= end) 
	{
		return NULL;
	}

	for (int j = i; j < i + 7; ++j) 
	{
		if (table[j / 8] & (1 << (7 - (j % 8))) > 0) 
		{
			break;
		}
		if ((table[j / 8] & (1 << (7 - (j % 8))) == 0) && ((j / 8) + 1) == end) 
		{
			return NULL;
		}
	}

	if ((table[i / 8] & (1 << (7 - (i % 8)))) > 0) 
	{
		//create leaf
		//read next 8 bits
		i++;
		char ch;
		unsigned int b = 0;
		for (int k = 7; k >= 0; --k) 
		{
			int b_shift = 7 - (i % 8);
			unsigned int table_bit = (table[i / 8] & (1 << b_shift)) > 0 ? 1 : 0;
			b |= table_bit << k;
			i++;
		}
		ch = b;
		Node* leaf = new Node(0, ch);
		i--; //backtrack index
		return leaf;
	}
	else 
	{
		//create parent and leaf nodes
		Node* parent = new Node(0, '\0');
		parent->right_child = rebuild_tree(table, end, ++i);
		parent->left_child = rebuild_tree(table, end, ++i);
		return parent;
	}
}

int main(int argc, char** argv) 
{
	//error handling for invalid arguements
	if (argc != 2) 
	{
		cerr << "Invalid number of arguements!" << endl;
		return 0;
	}

	//open file from arguements and create encoded .huf file
	ifstream infile(argv[1], ios::binary);
	ofstream outfile;
	string outfile_name = argv[1];
	outfile_name = outfile_name.substr(0, outfile_name.size() - 4);
	outfile.open(outfile_name.c_str(), ios::binary);

	//rebuild huff tree
	vector<unsigned char> buffer(
		(istreambuf_iterator<char>(infile)),
		(istreambuf_iterator<char>()));

	vector<unsigned char> table_buffer;
	int eot = 0;
	bool null_byte_clear = false;
	for (int i = 0; i < buffer.size(); ++i) 
	{
		//checking that the beginning doesnt lead with a byte of 0's
		if (!null_byte_clear && buffer[i] != '\0') 
		{
			null_byte_clear = true;
		}
		if (null_byte_clear && buffer[i] == '\0') 
		{
			eot = i + 1;
			break;
		}
		table_buffer.push_back(buffer[i]);
	}
	//rebuild huffman tree
	int rebuild_index = 0;
	Node* freq_heap = rebuild_tree(table_buffer, eot, rebuild_index);
	Node* head = freq_heap;

	int byte_count = 0;
	for (int i = eot; i < buffer.size(); ++i) 
	{
		for (int j = 0; j < 8; ++j) 
		{
			int b = (buffer[i] & (1 << j)) > 0 ? 1 : 0;
			if (b == 1 && head->left_child) 
			{
				head = head->left_child;
			}
			else if (b == 0 && head->right_child) 
			{
				head = head->right_child;
			}
			if (head->byte_code != '\0') 
			{
				outfile.put(static_cast<char>(head->byte_code));
				head = freq_heap;
				byte_count++;
			}
		}
	}

	delete freq_heap;

	infile.close();
	outfile.close();

	return 0;
}
