#include "user_input.h"

bool UserInput::GetUnsignedInt(uint64_t& output,
                               std::string prompt,
                               const std::set<uint64_t>* const valid_options,
                               std::vector<std::string>* const test_input)
{
    printf("%s",prompt.c_str());
    printf("\n");

    std::string input;
    int converted;

    bool testing;
    bool valid = false;
    std::set<std::string> options_set_string;
    std::vector<std::string> options_vector_string;

    if (test_input == NULL)
        testing = false;
    else
        testing = true;

    // Copy Contents of valid_options
    if (valid_options != NULL)
    {
        for (auto option : *valid_options)
        {
            options_set_string.insert(std::to_string(option));
            options_vector_string.push_back(std::to_string(option));
        }
    }

    // Get first input
    if (testing)
    {
        input = test_input->at(0);
        test_input->erase(test_input->begin());
    }
    else
        std::cin >> input;

    while (!(input == "q" || input == "Q" || valid))
    {
        valid = true;

        // check if it exists in the valid options set
        if (valid_options != NULL &&
            options_set_string.find(input) == options_set_string.end())
        {
            printf("Must be one of the following options:\n");
            for (int i = 0; i < options_vector_string.size(); i++)
            {
                printf(options_vector_string[i].c_str());
                if (i < (options_vector_string.size() - 1))
                    printf(", ");
            }
            printf("\nre-enter or \"q\" to quit:\n");
            valid = false;
        }

        // check for valid int conversion
        else if (!parse_text_.ConvertInt(input, converted))
        {
            printf("Must be an unsigned integer, re-enter or \"q\" to quit:\n");
            valid = false;
        }
        // check if it is a positive int
        else if (converted < 0)
        {
            printf("Must be a positive integer, re-enter or \"q\" to quit:\n");
            valid = false;
        }

        if (!valid)
        {
            if (testing)
            {
                input = test_input->at(0);
                test_input->erase(test_input->begin());
            }
            else
                std::cin >> input;
        }
    }

    if (valid)
        output = (uint64_t)converted;

    return valid;
}

bool UserInput::GetString(std::string& output,
                          std::string prompt,
                          const std::set<std::string>* const valid_options,
                          std::vector<std::string>* const test_input)
{
    printf("%s",prompt.c_str());
    printf("\n");

    std::string input;
    int converted;

    bool testing;
    bool valid = false;

    std::set<std::string> options_set_string;
    std::vector<std::string> options_vector_string;

    // Copy Contents of valid_options
    if (valid_options != NULL)
    {
        for (auto option : *valid_options)
        {
            options_set_string.insert(option);
            options_vector_string.push_back(option);
        }
    }

    if (test_input == NULL)
        testing = false;
    else
        testing = true;

    // Get first input
    if (testing)
    {
        input = test_input->at(0);
        test_input->erase(test_input->begin());
    }
    else
        std::cin >> input;

    while (!(input == "q" || input == "Q" || valid))
    {
        valid = true;

        // check if it exists in the valid options set
        if (valid_options != NULL &&
            options_set_string.find(input) == options_set_string.end())
        {
            printf("Must be one of the following options:\n");
            for (int i = 0; i < options_vector_string.size(); i++)
            {
                printf(options_vector_string[i].c_str());
                if (i < (options_vector_string.size() - 1))
                    printf(", ");
            }
            printf("\nre-enter or \"q\" to quit:\n");
            valid = false;
            if (testing)
            {
                input = test_input->at(0);
                test_input->erase(test_input->begin());
            }
            else
                std::cin >> input;
        }
    }

    output = input;
    return valid;
}
