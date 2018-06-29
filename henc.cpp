//STEPHEN COLUCCIO cs435 9136 mp
//Option 1: Huffman coding, encoding

#include <vector>
#include <string.h>
#include <fstream>
#include <iostream>
#include <bitset>

using namespace std;

class Node{
public: 
	int freq;
	char byte_code;
	Node * left_child;
	Node * right_child;

	Node(int f, char byte='\0', Node* left=NULL, Node* right=NULL): 
		freq(f), byte_code(byte), left_child(left), right_child(right) {}
	
	~Node()
	{
		delete left_child;
		delete right_child;
	}
	
};


//Priorty Heap
class PriorityHeap{
public:
	vector<Node*> heap;

	PriorityHeap(){}
	~PriorityHeap(){}

	void insert(Node* element){
		heap.push_back(element);
		heapify_up(heap.size()-1);
	}

	Node* extract_min(){
		Node* min = heap.front();
		heap[0] = heap.at(heap.size()-1);
		heap.pop_back();
		heapify_down(0);
		return min;
	}

	void heapify_up(int idx){
		while(parent(idx) >= 0 && idx > 0 && heap[parent(idx)]->freq > heap[idx]->freq){
			Node* temp = heap[parent(idx)];
			heap[parent(idx)] = heap[idx];
			heap[idx] = temp;
			idx = parent(idx);
		}
	}

	void heapify_down(int idx){
		int child = left(idx);
		if(child > 0 && right(idx) > 0 && heap[child]->freq > heap[right(idx)]->freq){
			child = right(idx);
		} 
		if(child > 0){
			Node* temp = heap[idx];
			heap[idx] = heap[child];
			heap[child] = temp;
			heapify_down(child);
		}
	}
	
	//parent index
	int parent(int child){
		if(child != 0){
			int idx = (child-1)/2;
			return idx;
		}
		return -1;
	}
	
	//left child index
	int left(int parent){
		int idx = (parent*2) + 1;
		return idx < heap.size() ? idx : -1;
	}

	//right child index
	int right(int parent){
		int idx = (parent*2) + 2;
		return idx < heap.size() ? idx : -1;
	}
};

//Write the construction of the frequency heap. 
//Internal Node : 0
//Leaf Node : 1 followed by char of node
void write_freq_heap(Node* node, string& table_str){
	if(node){
		if(node->byte_code == '\0'){
			table_str += "0";
			write_freq_heap(node->right_child, table_str);
			write_freq_heap(node->left_child, table_str);
		}else{
			table_str += "1";
			table_str += (bitset<8>(node->byte_code)).to_string();
		}
	}
}


//encode bit pattern to be written to file
void encode_bit_string(Node* node, unsigned char ch, string bit_str, vector<string>& local_bits){
	if(node){
		if(node->byte_code == ch && !node->right_child && !node->left_child){
			local_bits[ch] = bit_str;
		}else{
			encode_bit_string(node->right_child, ch, bit_str+"0", local_bits);
			encode_bit_string(node->left_child, ch, bit_str+"1", local_bits);
		}
	}
}




int main(int argc, char** argv){
	
	//error handling for invalid arguements
	if(argc != 2){
		cerr << "Invalid number of arguements!" << endl;
		return 0;
	}
	
	//open file from arguements and create encoded .huf file
	ifstream infile(argv[1], ios::in | ios::binary);
	vector<unsigned char> buffer (
		(istreambuf_iterator<char>(infile)), 
		(istreambuf_iterator<char>()));
	
	//create output file
	ofstream outfile;
	outfile.open(strcat(argv[1],".huf"), ios::binary);
	
	//determine frequency of bytes in provided file
	vector<int> frequency(256);
	for(int i=0; i<buffer.size(); ++i){
		frequency[(int)buffer[i]]++;
	}

	//create priority heap
	PriorityHeap* freq_heap = new PriorityHeap();
	for(int i=0; i<frequency.size(); ++i){
		if(frequency[i] != 0){
			freq_heap->insert(new Node(frequency[i], i));
		}
	}

	//build huffman tree
	while(freq_heap->heap.size() > 1){
		Node* right = freq_heap->extract_min();
		Node* left = freq_heap->extract_min();
		Node* parent_node = new Node(
			right->freq + left->freq,
			'\0', 
			left, right);
	
		freq_heap->insert(parent_node);
	}

	//write huffman tree to encoded file
	string table_str;
	write_freq_heap(freq_heap->heap[0], table_str);
	
	unsigned char write_ch;
	int bit_count=7;
	for(int i=0; i<table_str.size(); ++i){
		unsigned int bit = table_str[i] == '1' ? 1 : 0;
		write_ch |= bit << bit_count;
		if(bit_count == 0){
			outfile.put(write_ch);
			write_ch = '\0';
			bit_count = 8;
		}
		bit_count--;
		bit_count %= 8;
	}
	if(write_ch != '\0') outfile.put(write_ch);
	//write a delimiter to know end of huff tree
	outfile.put('\0');

	//create huffman codes
	vector<string> huff_codes(256);
	unsigned char ch;
	for(int i=0; i<256; ++i){
		huff_codes[i] = "";
		ch = static_cast<unsigned char>(i);
		encode_bit_string(freq_heap->heap[0], ch, "", huff_codes);
	}

	//write huffman codes to outfile
	unsigned char encoded_ch;
	for(int i=0; i<buffer.size(); ++i){
		ch = buffer[i];
		int bit_count;
		for(unsigned int j=0; j<huff_codes[ch].size(); ++j){
			unsigned int bit_str = huff_codes[ch].at(j) == '1' ? 1 : 0;
			encoded_ch |= bit_str << bit_count;
			bit_count++;
			bit_count %= 8;
			if(bit_count == 0){
				outfile.put(static_cast<char>(encoded_ch));
				encoded_ch = '\0';
			}
		}
	}
	//write the remaining bits if they exist
	if(encoded_ch != '\0') outfile.put(encoded_ch);
	
	infile.close();
	outfile.close();

	delete freq_heap;

	return 0;
}
