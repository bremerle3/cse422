#include <stdio.h>
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <map>
#include <fstream>

int main (int argc, char* argv[])
{
    //Program exit values 
    const int NO_ERROR = 0;
    const int CLA_ERROR = 1;
    const int FILE_ERROR = 2;
    int flag_error; 
    //File IO values
    const int FIRST_ARG = 1;
    const int LAST_ARG = argc-1;
    int fflag = -1; //Use -1 as sentinel value.
    int wflag = -1;
    int expected_args = -1; 

    //Buffers for input args and text filtering.
    std::vector<std::string> dict; //This will store the dictionary (could be one word).
    std::vector<std::string> text_vec;  //This will store the tokenized input text.
    std::map<std::string, int> text_map;  //Maps strings to number of occurences.
    char *text_name = NULL;   //Name of text file to search. 
    char *dict_name = NULL;   //Name of dictionary file. 
    char *fout = NULL;   //Name of output file. 
    char *text = NULL;  //Buffer to dump the text into.
    std::ifstream file_in;  //Text to parse
    std::ofstream file_out;  //Output file
    text_name = argv[LAST_ARG];  
    if(argc == 3)
    {
        dict.push_back(argv[FIRST_ARG]);
    }
    else
    {
        fflag = -1; //Use -1 as sentinel value.
        wflag = -1;
        expected_args = -1; 
        for(int i=1; i<argc; i++) //argv[0] is program name, don't bother with it.
        {
            if(!strcmp(argv[i], "-f"))
            {
                fflag = i;
            }
            else if(!strcmp(argv[i], "-w"))
            {
                wflag = i;
            }
        }
        //Should always have program name and input file name.
        //Could have 1, 2, 3, or 4 more args depending on flags.
        expected_args = 2 + ((fflag>0) ? 2 : 1) + ((wflag>0) ? 2 : 0);
        if(expected_args != argc)
        {
            std::cout << "Error: wrong number of command line arguments." << std::endl;
            return CLA_ERROR;
        }
        else 
        {
            if(fflag>0)
            {
                dict_name = argv[fflag+1];
                //I hope 128 bytes is enough for any word...if not, have to do heap
                //allocations. I didn't bother.
                int buf_size = 128;
                char buf[buf_size];  
                FILE *dfile = fopen(dict_name, "r");
                if(dfile == NULL)
                {
                    std::cout << "Error opening dictionary file." << std::endl;
                    return FILE_ERROR;
                }
                else
                {
                    while(fgets(buf,buf_size, dfile))
                    {
                        buf[strcspn(buf, "\n\r")] = 0;
                        dict.push_back(buf);
                    }
                }
            }
            //If a dictionary file was not supplied, search word is the first argument.
            else
            {
                dict.push_back(argv[FIRST_ARG]);
            }
            if(wflag>0)
            {
                fout = argv[wflag+1];
                file_out.open(fout);
                if(!file_out.is_open())
                {
                    std::cout << "Error opening output file." << std::endl;
                    return FILE_ERROR;
                }
            }
        }
    }
    //Read the whole file
    int length;
    file_in.open(text_name); 
    file_in.seekg(0, std::ios::end);
    length = file_in.tellg();       
    file_in.seekg(0, std::ios::beg);
    text = new char[length];   
    file_in.read(text, length);     
    file_in.close();
    //Convert to C++ style string
    std::string text_str(text);
    //Get rid of UTF char —, which strtok can't handle.
    std::size_t found = text_str.find("—");
    std::string annoying_char = "—";
    while(found != std::string::npos)
    {
        text_str.erase(found, annoying_char.size()-1);
        text_str.replace(found, 1, " ");
        found = text_str.find("—"); 
    }
    //Change back to C-style string for strtok
    char text_filtered[text_str.size()];
    strncpy(text_filtered, text_str.c_str(), text_str.size()); 
    //std::cout << "text: " << text << std::endl;    

    //Tokenize the file.
    char * pch;
    pch = strtok (text_filtered," \"\n\r,.;:-?!()");
    while (pch != NULL)
    {
        std::string str = std::string(pch);
        //Annoyingly, A-Trip-To-Mars uses ’ instead of ". Remove them 
        //from the beginning of chars. End is OK since another delimiter
        //will appear before it, but that will leave the ’ by itself.  
        if(str != "’") //I don't consider ’ by itself to be a word.
        {
            if(str.size() >= 3 && str.substr(0,3) == "’")
            {
                str = str.substr(3, str.size()-3);
            }
            if(str.size() >= 3 && str.substr(0,3) == "‘")
            {
                str = str.substr(3, str.size()-3);
            }
            if(str.size() >= 3 && str.substr(0,3) == "“")
            {
                str = str.substr(3, str.size()-3);
            }
            if(str.size() >= 3 && str.substr(0,3) == "”")
            {
                str = str.substr(3, str.size()-3);
            }
            if(str.size() >= 3 && str.substr(0,3) == "—")
            {
                str = str.substr(3, str.size()-3);
            }
            if(str.size() >= 3 && str.substr(str.size()-3, 3) == "’")
            {
               str = str.substr(0, str.size()-3);
            } 
            text_vec.push_back(str);
            //std::cout << str << std::endl;
            //std::cout << "size: " << str.size() << std::endl;
        };
        pch = strtok (NULL, " \"\n\r,.;:-?!()");
    }


    //Zeroize the map
    for(std::vector<std::string>::size_type i=0; i != text_vec.size(); i++)
    {
        text_map[text_vec[i]] = 0;        
    }
    //Map each word to a number of occurences.
    for(std::vector<std::string>::size_type i=0; i != text_vec.size(); i++)
    {
        text_map[text_vec[i]]++;        
    }

    //Print result
    //for(std::map<std::string, int>::const_iterator it = text_map.begin(); it != text_map.end(); ++it)
    //{
    //    std::cout << it->first << "," << it->second << std::endl;
    //}
   
    for(int i=0; i<dict.size(); i++)
    {
        //Search the map for each element in the dictionary.
        std::map<std::string, int>::iterator it = text_map.find(dict.at(i));
        if(it != text_map.end())
        {
            if(wflag>0)
            {
                file_out << dict.at(i) << "," << text_map.find(dict.at(i))->second << std::endl;
            }
            else
            {
                std::cout << dict.at(i) << "," << text_map.find(dict.at(i))->second << std::endl;
            }
        }
    }

    //Clean up.
    file_in.close();
    file_out.close();
    return NO_ERROR;
}





