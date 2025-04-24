#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "my_string.h"
#include "generic_vector.h"
#include "associative_array.h"

int get_word_length(void);
int get_number_of_guesses(void);
char getGuess(void);
Boolean check_guess(MY_STRING past_guesses, char current_guess);
char play_again(void);
void clear_keyboard_buffer();
void evil_hangman(int word_length, int number_of_guesses);

int main(int argc, char* argv[]) {
    int word_length;
    int number_of_guesses;

    do {
        printf("EVIL HANGMAN GAME !!!\n\n");

        word_length = get_word_length();
        number_of_guesses = get_number_of_guesses();

        evil_hangman(word_length, number_of_guesses);

    } while (play_again() == 'y');

    return 0;
}

int get_word_length(void) {
    int word_length;

    do {
        printf("What word length would you like to start with? ");
        scanf("%d", &word_length);
        clear_keyboard_buffer();

        if (word_length < 2 || word_length > 29 || word_length == 27 || word_length == 26 || word_length == 23 || word_length == 25) {
            printf("Invalid word length! Please try again.\n");
        } else {
            return word_length;
        }
    } while (1);
}

int get_number_of_guesses(void) {
    int number_of_guesses;

    do {
        printf("How many guesses would you like to have: ");
        scanf("%d", &number_of_guesses);
        clear_keyboard_buffer();

        if (number_of_guesses <= 0) {
            printf("You must enter a positive number of guesses.\n");
        } else {
            return number_of_guesses;
        }
    } while (1);
}

void evil_hangman(int word_length, int number_of_guesses) {
    FILE* fp = fopen("dictionary_real.txt", "r");
    if (!fp) {
        fprintf(stderr, "Failed to open dictionary file.\n");
        return;
    }

    GENERIC_VECTOR current_word_list = generic_vector_init_default(my_string_assignment, my_string_destroy);
    if (!current_word_list) {
        fprintf(stderr, "Failed to initialize word list. Exiting...\n");
        fclose(fp);
        return;
    }

    MY_STRING temp_word = my_string_init_default();
    while (my_string_extraction(temp_word, fp) == SUCCESS) {
        if (my_string_get_size(temp_word) == word_length) {
            MY_STRING word_copy = my_string_init_c_string(my_string_c_str(temp_word));
            if (word_copy) {
                generic_vector_push_back(current_word_list, word_copy);
            }
        }
    }
    fclose(fp);
    my_string_destroy(&temp_word);

    if (generic_vector_get_size(current_word_list) == 0) {
        printf("The Computer has 0 possibilities remaining.\n");
        printf("Try again with a different word length.\n");
        generic_vector_destroy(&current_word_list);
        return;
    }

    MY_STRING key = my_string_init_default();
    for (int i = 0; i < word_length; i++) {
        my_string_push_back(key, '-');
    }

    MY_STRING guessed_letters = my_string_init_default();
    TREE family_tree = tree_init_default();

    while (number_of_guesses > 0 && my_string_c_str(key) != NULL && strchr(my_string_c_str(key), '-') != NULL) {
        printf("\nYou have %d guesses left.\n", number_of_guesses);
        printf("Used Letters: %s\n", my_string_get_size(guessed_letters) > 0 ? my_string_c_str(guessed_letters) : "(none)");
        printf("Word: %s\n", my_string_c_str(key));
        printf("Please guess a character: ");

        char guess;
        scanf(" %c", &guess);
        guess = tolower(guess);
        clear_keyboard_buffer();

        if (strchr(my_string_c_str(guessed_letters), guess)) {
            printf("You already guessed '%c'. Try another letter.\n", guess);
            continue;
        }

        my_string_push_back(guessed_letters, guess);
        tree_destroy(&family_tree);
        family_tree = tree_init_default();

        for (int i = 0; i < generic_vector_get_size(current_word_list); i++) {
            MY_STRING word = (MY_STRING)generic_vector_at(current_word_list, i);
            MY_STRING new_key = my_string_init_default();
            get_word_key_value(key, new_key, word, guess);
            tree_add(family_tree, new_key, word);
            my_string_destroy(&new_key);
        }

        GENERIC_VECTOR largest_family = tree_largest_word_bin(family_tree);
        if (!largest_family || generic_vector_get_size(largest_family) == 0) {
            printf("No words matched. This shouldn't happen. Exiting.\n");
            break;
        }

        MY_STRING sample = (MY_STRING)generic_vector_at(largest_family, 0);
        MY_STRING new_key = my_string_init_default();
        get_word_key_value(key, new_key, sample, guess);

        Boolean correct = FALSE;
        for (int i = 0; i < word_length; i++) {
            if (my_string_c_str(key)[i] != my_string_c_str(new_key)[i]) {
                correct = TRUE;
                break;
            }
        }

        if (!correct) {
            printf("Sorry, there are no '%c's.\n", guess);
            number_of_guesses--;
        } else {
            printf("Good guess!\n");
        }

        my_string_destroy(&key);
        key = new_key;
        generic_vector_destroy(&current_word_list);
        current_word_list = largest_family;
    }

    if (strchr(my_string_c_str(key), '-') == NULL) {
        printf("\nCongratulations! You guessed the word: %s\n", my_string_c_str(key));
    } else {
        MY_STRING answer = (MY_STRING)generic_vector_at(current_word_list, 0);
        printf("\nSorry, you lost. The word was: %s\n", my_string_c_str(answer));
    }

    my_string_destroy(&key);
    my_string_destroy(&guessed_letters);
    generic_vector_destroy(&current_word_list);
    tree_destroy(&family_tree);
}



char play_again(void) {
    char choice;
    printf("Would you like to play again? (y/n): ");
    scanf(" %c", &choice);
    clear_keyboard_buffer();
    return tolower(choice);
}

void clear_keyboard_buffer() {
    char ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
}
