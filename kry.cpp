#include <string>
#include <iostream>
#include <map>
#include <getopt.h>
#include <algorithm>
#include <fstream>
#include <stdlib.h>
#include <vector>

std::vector<int> a_possibilities{1, 3, 5, 7, 9, 11, 15, 17, 19, 21, 23, 25};

std::map<int, int> inversions = {
    { 1, 1  },
    { 3, 9  },
    { 5, 21 },
    { 7, 15 },
    { 9, 3  },
    { 11, 19},
    { 15, 7 },
    { 17, 23},
    { 19, 11},
    { 21, 5 },
    { 23, 17},
    { 25, 25}
};

float czech_frequency[26] = {   0.08827, 0.01665, 0.02618, 0.03632, 0.105,   0.00394, // A-F
                                0.00343, 0.01296, 0.07674, 0.01983, 0.03752, 0.04097, // G-L
                                0.03262, 0.06749, 0.08315, 0.03454, 0.00006, 0.05157, // M-R
                                0.05437, 0.05592, 0.03845, 0.04378, 0.00072, 0.00092, // S-X
                                0.02694, 0.03145                                      // Y-Z          
                            };

int getIndex(char c){
    return (int)c - 65;
}

char getLetter(int i){
    return (char)i + 65;
}

std::string encrypt(std::string message, int a, int b){
    std::string result = "";

    for(char letter : message){

        if (letter == ' ')
        {
            char new_letter = ' ';
            result += new_letter;
        }
        else
        {
            int idx = getIndex(letter);
            int new_index = (a * idx + b) % 26;
            char new_letter = getLetter(new_index);
            result += new_letter;
        }
    }

    return result;
}

std::string decrypt(std::string message, int a, int b){
    std::string result = "";

    for(char letter : message){

        if (letter == ' ')
        {
            char new_letter = ' ';
            result += new_letter;
        }
        else
        {
            int idx = getIndex(letter);
            int new_index = (((inversions[a] * (idx - b)) % 26) + 26) % 26;          
            char new_letter = getLetter(new_index);
            result += new_letter;
        }
    }

    return result;
}

std::string crack(std::string message){

    // Tato funkce provadi odhad parametru a,b na zaklade frekvencni analyzy
    float histogram[26] = { 0 };
    int nb_chars = 0;

    for(char letter : message){

        if (letter != ' ')
        {
            histogram[getIndex(letter)]++;
            nb_chars++;
        }     
    }

    for (int i=0 ; i<26; i++ )
    {
        histogram[i] /= nb_chars;
    }

    int cz_letter_e_idx = 4;    // index nejcastejsiho pismene v cestine
    int cz_letter_a_idx = 0;    // index druheho nejacastejsiho pismene v cestine


    float histogram_copy[26];
    std::copy (histogram, histogram+26, histogram_copy);


    int expected_e_idx = std::distance(histogram, std::max_element(histogram, histogram + 26));

    std::nth_element(histogram_copy, histogram_copy + 1, histogram_copy + 26, std::greater<float>());    
    int expected_a_idx = std::distance(histogram, std::find(histogram, histogram + 26, histogram_copy[1]));

    int a_expected, b_expected = 0;

    int c1 = expected_a_idx;
    int c2 = expected_e_idx;

    int p1 = cz_letter_e_idx;
    int p2 = cz_letter_a_idx;


    // Parametry a,b musi splnovat nasledujici rovnice
    // c1 == p1 * a + b % 26;
    // c2 == p2 * a + b % 26;
   
    for (int a : a_possibilities){
        for (int b = 0; b < 26; b++){
            if ((c1 == ((p1 * a) + b) % 26) && (c2 == ((p2 * a) + b) % 26))
            {
                a_expected = a;
                b_expected = b;  
                goto jump;
            }
        }
    }

    jump:
    std::cout << "a=" << a_expected << ",b=" << b_expected << std::endl;

    return decrypt(message, a_expected, b_expected);
    
}

std::string crack_BF(std::string message){

    int best_a, best_b = 0;
    float best_loss = 1000000000000.0;


    for (int a : a_possibilities){
        for (int b = 0; b < 26; b++){
            auto result = decrypt(message, a, b);

            float histogram[26] = { 0 };
            int nb_chars = 0;

            for(char letter : result){

                if (letter != ' ')
                {
                    histogram[getIndex(letter)]++;
                    nb_chars++;
                }     
            }

            for (int i=0 ; i<26; i++ )
            {
                histogram[i] /= nb_chars;
            }

            float loss = 0.0;
            for (int i = 0; i < 26; i++)
            {
                loss += abs(histogram[i] - czech_frequency[i]);
            }           

            if (loss < best_loss)
            {
                best_a = a;
                best_b = b;
                best_loss = loss;
            }
        }
    }

    std::cout << "a=" << best_a << ",b=" << best_b << std::endl;

    return decrypt(message, best_a, best_b);
}

int main(int argc, char *argv[]){
    
    int opt, a, b;
    char operation = 'x';
    std::string input_file_name, output_file_name;
  
    while((opt = getopt(argc, argv, "edca:b:f:o:")) != -1) 
    { 
        switch(opt) 
        { 
            case 'e':
                operation = 'e';
                break;
            case 'd':
                operation = 'd';
                break;
            case 'c':
                operation = 'c';
                break;
            case 'a': 
                a = atoi(optarg);
                break; 
            case 'b': 
                b = atoi(optarg);
                break;
            case 'f':
                input_file_name = optarg;
                break;
            case 'o':
                output_file_name = optarg;
                break;
            case '?': 
                printf("unknown option: %c\n", optopt);
                break;
            default:
                printf("unknown option: %c\n", optopt);
                break;
        } 
    }

    std::string message = "";

    if (optind < argc)
    {
        message = argv[optind];
    }

    std::transform(message.begin(), message.end(), message.begin(), ::toupper);

    if (operation == 'e')
    {
        std::cout << encrypt(message, a, b) << std::endl;
    }
    else if (operation == 'd')
    {
        std::cout << decrypt(message, a, b) << std::endl;
    }
    else if (operation == 'c')
    {
        std::string line;
        std::ifstream input_file(input_file_name);
        if (input_file.is_open())
        {
            getline(input_file, line);            
            input_file.close();
            // auto result = crack(line);
            auto result = crack_BF(line);     
            std::ofstream output_file(output_file_name);
            if (output_file.is_open())
            {
                output_file << result << std::endl;
            }
            else
            {
                std::cerr << "Unable to open output file";
            }          
        }
        else
        {
            std::cerr << "Unable to open input file";
        } 
    }
    else
    {
        std::cerr << "Unknown operation" << std::endl;
    }
           
    return 0;
}