
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <bitset>
#include <stdlib.h>

#define HEAD 1145128264 // Run begin buffer
#define DATA 1096040772 // Physics data buffer
#define SCAL 1279345491 // Scaler type buffer
#define DEAD 1145128260 // Deadtime buffer
#define DIR 542263620   // "DIR "
#define PAC 541278544   // "PAC "
#define ENDFILE 541478725 // End of file buffer
#define ENDBUFF -1 // End of buffer marker

struct CLoption{
	char opt;
	std::string alias;
	std::string value;
	bool require_arg;
	bool optional_arg;
	bool is_active;
	
	CLoption(){
		Set("NULL", false, false);
		opt = 0x0;
		value = "";
		is_active = false;
	}
	
	CLoption(std::string name_, bool require_arg_, bool optional_arg_){
		Set(name_, require_arg_, optional_arg_);
		value = "";
		is_active = false;
	}
	
	void Set(std::string name_, bool require_arg_, bool optional_arg_){
		opt = name_[0]; alias = name_; require_arg = require_arg_; optional_arg = optional_arg_;
	}
};

/* Print help dialogue for command line options. */
void help(){
	std::cout << "\n SYNTAX: ./HexRead [filename] [options]\n";
	std::cout << "  -t, --type <int>   Only display buffers of a certain type\n"; 
	std::cout << "  -r, --raw          Display raw buffer words (hidden by default)\n";
	std::cout << "  -c, --convert      Attempt to convert words to Ascii characters\n";
	std::cout << "  -s, --search <int> Search for an integer in the stream\n";
	std::cout << "  -z, --zero         Suppress zero output\n\n";
	std::cout << " Available Buffer Types:\n";
	std::cout << "  \"HEAD\" 1145128264\n";
	std::cout << "  \"DATA\" 1096040772\n"; // Physics data buffer
	std::cout << "  \"SCAL\" 1279345491\n"; // Scaler type buffer
	std::cout << "  \"DEAD\" 1145128260\n"; // Deadtime buffer
	std::cout << "  \"DIR \" 542263620\n";  // "DIR "
	std::cout << "  \"PAC \" 541278544\n";  // "PAC "
	std::cout << "  \"EOF \" 541478725\n\n";       // End of buffer marker
}

/* Extract a string from a character array. */
std::string csubstr(char *str_, unsigned int start_index_=0){
	std::string output = "";
	unsigned int index = start_index_;
	while(str_[index] != '\0' && str_[index] != ' '){
		output += str_[index];
		index++;
	}
	return output;
}

/* Parse all command line entries and find valid options. */
bool get_opt(int argc_, char **argv_, CLoption *options, unsigned int num_valid_opt_, unsigned int start_index_=1){
	unsigned int index = start_index_;
	unsigned int previous_opt;
	bool need_an_argument = false;
	bool may_have_argument = false;
	bool is_valid_argument = false;
	while(index < argc_){
		if(argv_[index][0] == '-'){
			if(need_an_argument){
				std::cout << " Error: --" << options[previous_opt].alias << " [-" << options[previous_opt].opt << "] requires an argument\n";
				help();
				return false;
			}

			is_valid_argument = false;
			if(argv_[index][1] == '-'){ // Word options
				std::string word_arg = csubstr(argv_[index], 2);
				for(unsigned int i = 0; i < num_valid_opt_; i++){
					if(word_arg == options[i].alias && word_arg != "NULL"){
						options[i].is_active = true; 
						previous_opt = i;
						if(options[i].require_arg){ need_an_argument = true; }
						else{ need_an_argument = false; }
						if(options[i].optional_arg){ may_have_argument = true; }
						else{ may_have_argument = false; }
						is_valid_argument = true;
					}
				}
				if(!is_valid_argument){
					std::cout << " Error: encountered unknown option --" << word_arg << std::endl;
					help();
					return false;
				}
			}
			else{ // Character options
				unsigned int index2 = 1;
				while(argv_[index][index2] != '\0'){
					for(unsigned int i = 0; i < num_valid_opt_; i++){
						if(argv_[index][index2] == options[i].opt && argv_[index][index2] != 0x0){ 
							options[i].is_active = true; 
							previous_opt = i;
							if(options[i].require_arg){ need_an_argument = true; }
							else{ need_an_argument = false; }
							if(options[i].optional_arg){ may_have_argument = true; }
							else{ may_have_argument = false; }
							is_valid_argument = true;
						}
					}
					if(!is_valid_argument){
						std::cout << " Error: encountered unknown option -" << argv_[index][index2] << std::endl;
						help();
						return false;
					}
					index2++;
				}
			}
		}
		else{ // An option argument
			if(need_an_argument || may_have_argument){
				options[previous_opt].value = csubstr(argv_[index]);
				need_an_argument = false;
				may_have_argument = false;
			}
			else{
				std::cout << " Error: --" << options[previous_opt].alias << " [-" << options[previous_opt].opt << "] takes no argument\n";
				help();
				return false;			
			}
		}
		
		// Check for the case where the end option requires an argument, but did not receive it
		if(index == argc_-1 && need_an_argument){
			std::cout << " Error: --" << options[previous_opt].alias << " [-" << options[previous_opt].opt << "] requires an argument\n";
			help();
			return false;	
		}
		
		index++;
	}
	return true;
}

std::string convert_to_hex(int input_, bool to_text_=false){
    std::bitset<32> set(input_);  
    std::stringstream stream;
    if(!to_text_){ stream << std::hex << std::uppercase << set.to_ulong(); }
    else{ stream << std::uppercase << set.to_ulong(); }
    std::string output = stream.str();
    if(!to_text_ && output.size() < 8){
    	std::string temp = "0x";
    	for(unsigned int i = output.size(); i < 8; i++){
    		temp += '0';
    	}
    	return temp + output;
    }
    else if(to_text_ && output.size() < 10){
    	std::string temp = "";
    	for(unsigned int i = output.size(); i < 10; i++){
    		temp += ' ';
    	}
    	return temp + output;
    }
    
    if(!to_text_){ return "0x" + output; }
    return output;
}

int main(int argc, char *argv[]){
	if(argc < 2){
		help();
		return 1;
	}
	
	CLoption valid_opt[5];
	valid_opt[0].Set("type", true, false);
	valid_opt[1].Set("raw", false, false);
	valid_opt[2].Set("convert", false, false);
	valid_opt[3].Set("search", true, false);
	valid_opt[4].Set("zero", false, false);
	if(!get_opt(argc, argv, valid_opt, 5, 2)){ return 1; }
	
	std::ifstream input(argv[1], std::ios::binary);
	if(!input.is_open()){
		std::cout << " Error: failed to open input file\n";
		return 1;
	}

	int buffer_select = 0;
	int search_int = 0;
	bool show_raw = false;
	bool convert = false;
	bool show_zero = true;
	bool do_search = false;
	if(valid_opt[0].is_active){
		buffer_select = atoi(valid_opt[0].value.c_str());
		if(!(buffer_select == HEAD || buffer_select == DATA || buffer_select == SCAL || buffer_select == DEAD || buffer_select == DIR || buffer_select == PAC || buffer_select == ENDFILE)){
			std::cout << " Error: " << convert_to_hex(buffer_select) << " is not a valid buffer\n";
			buffer_select = 0;
		}
		else{ std::cout << " Displaying only buffer type " << convert_to_hex(buffer_select) << "\n"; }
	}
	if(valid_opt[1].is_active){ show_raw = true; }
	if(valid_opt[2].is_active){ convert = true; }
	if(valid_opt[3].is_active){ 
		do_search = true; 
		search_int = atoi(valid_opt[3].value.c_str());
		std::cout << " Searching for " << search_int << " (" << convert_to_hex(search_int) << ")\n";
	}
	if(valid_opt[4].is_active){ show_zero = false; }
	show_zero = true;

	int word;
	bool good_buffer;
	unsigned int count = 0;
	unsigned int word_count = 0;
	unsigned int total_count = 0;
	unsigned int buff_count = 0;
	unsigned int good_buff_count = 0;
	
	if(buffer_select != 0){ good_buffer = false; }
	else{ good_buffer = true; }
	
	int show_next = 0;
	while(true){
		input.read((char*)&word, 4);
		if(input.eof()){ 
			if(buff_count > 1){
				std::cout << " Buffer Size: " << word_count << " words\n";
				std::cout << "============================================================================================================================\n";
			}
			break; 
		}

		if(do_search){
			if(show_next > 0){
				std::cout << convert_to_hex(word) << "  ";
				if(convert){ std::cout << convert_to_hex(word, true) << "  "; }
				show_next = show_next - 1;
				if(show_next <= 0){ std::cout << std::endl; }
			}		
			else if(word == search_int){
				std::cout << convert_to_hex(word) << "  ";
				if(convert){ std::cout << convert_to_hex(word, true) << "  "; }
				show_next = 4;
			}
			total_count++;
			word_count++;
			continue;
		}

		// Check for end of buffer
		/*if(word == ENDBUFF || (!show_zero && word == 0)){
			total_count++;
			continue;
		}*/
		if((!show_zero && word == 0)){
			total_count++;
			word_count++;
			continue;
		}
				
		// Check for a new buffer
		if(word == HEAD || word == DATA || word == SCAL || word == DEAD || word == DIR || word == PAC || word == ENDFILE){ // new buffer
			buff_count++;
			if(buffer_select != 0){
				if(word == buffer_select){ good_buffer = true; }
				else{ good_buffer = false; }
			}
			
			if(good_buffer){
				good_buff_count++;
				if(buff_count > 1){
					if(show_raw){ std::cout << "\n"; }
					std::cout << "\n Buffer Size: " << word_count << " words\n";
					std::cout << "============================================================================================================================\n";
				}
				std::cout << "\n============================================================================================================================\n";
				std::cout << " Buffer Number: " << buff_count << std::endl;
				std::cout << " Buffer Type: " << convert_to_hex(word);
				if(word == HEAD){ std::cout << " \"HEAD\"\n"; }
				else if(word == DATA){ std::cout << " \"DATA\"\n"; }
				else if(word == ENDFILE){ std::cout << " \"EOF \"\n"; }
				else if(word == SCAL){ std::cout << " \"SCAL\"\n"; }
				else if(word == DEAD){ std::cout << " \"DEAD\"\n"; }
				else if(word == DIR){ std::cout << " \"DIR\"\n"; }
				else if(word == PAC){ std::cout << " \"PAC\"\n"; }
				std::cout << " Total Count: " << total_count << " words\n";
				count = 0;
			}
			word_count = 0; // New buffer. Reset the buffer word count
		}
		
		if(!good_buffer){ // We don't care about this buffer
			total_count++;
			word_count++;
			continue; 
		} 

		total_count++;
		word_count++;
		if(show_raw){
			if(count == 0){ std::cout << "\n0000  "; }
			else if(count % 10 == 0){ 
				std::stringstream stream; stream << count;
				std::string temp_string = stream.str();
				std::string padding = "";
				if(temp_string.size() < 4){
					for(unsigned int i = 0; i < (4 - temp_string.size()); i++){ 
						padding += "0";
					}
				}
				std::cout << "\n" << padding << temp_string << "  "; 
			}
			std::cout << convert_to_hex(word) << "  "; count++;
			if(convert){ std::cout << convert_to_hex(word, true) << "  "; count++; }
		}
	}
	input.close();
	
	if(!do_search){
		std::cout << "\n\n Read " << total_count << " 4 byte words (" << total_count*4 << " bytes)\n";
		std::cout << "  Found " << buff_count << " total buffers\n";
		if(buffer_select != 0){ std::cout << "  Found " << good_buff_count << " " << convert_to_hex(buffer_select) << " buffers\n"; }
	}
	
	return 0;
}