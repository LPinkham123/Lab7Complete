// main.c

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "my_string.h"
#include "generic_vector.h"
#include "associative_array.h"

int  get_word_length(void);
int  get_number_of_guesses(void);
char play_again(void);
void clear_keyboard_buffer(void);
void evil_hangman(int word_length, int number_of_guesses);

int main(int argc, char* argv[]) {
    int word_length;
    int number_of_guesses;

    do {
        printf("EVIL HANGMAN GAME !!!\n\n");
        word_length        = get_word_length();
        number_of_guesses  = get_number_of_guesses();
        evil_hangman(word_length, number_of_guesses);
    } while (play_again() == 'y');
    printf("iter 10");
    return 0;
}

int get_word_length(void) {
    int word_length;
    do {
        printf("What word length would you like to start with? ");
        scanf("%d", &word_length);
        clear_keyboard_buffer();
        if (word_length < 2 || word_length > 29 ||
            word_length == 23 || word_length == 25 ||
            word_length == 26 || word_length == 27) {
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

char play_again(void) {
    char choice;
    printf("Would you like to play again? (y/n): ");
    scanf(" %c", &choice);
    clear_keyboard_buffer();
    return tolower(choice);
}

void clear_keyboard_buffer(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) { }
}

void evil_hangman(int word_length, int number_of_guesses) {
    FILE*           fp                = NULL;
    GENERIC_VECTOR  dict_list         = NULL;  // Full dictionary of words
    GENERIC_VECTOR  current_list      = NULL;  // Shrinks each round
    MY_STRING       temp_word         = NULL;
    MY_STRING       key               = NULL;
    MY_STRING       guessed_letters   = NULL;
    TREE            family_tree       = NULL;

    /* 1) Open the dictionary file */
    fp = fopen("dictionary_real.txt", "r");
    if (!fp) {
        fprintf(stderr, "Failed to open dictionary file.\n");
        goto cleanup;
    }

    /* 2) Read every word of the right length into dict_list */
    dict_list = generic_vector_init_default(my_string_assignment, my_string_destroy);
    if (!dict_list) {
        fprintf(stderr, "Could not create dictionary vector.\n");
        goto cleanup;
    }

    temp_word = my_string_init_default();
    if (!temp_word) {
        fprintf(stderr, "Could not allocate temp_word.\n");
        goto cleanup;
    }

    while (my_string_extraction(temp_word, fp) == SUCCESS) {
        if (my_string_get_size(temp_word) == word_length) {
            MY_STRING word_copy = my_string_init_c_string(my_string_c_str(temp_word));
            if (word_copy) {
                // push into the vector (which makes its own internal copy)
                generic_vector_push_back(dict_list, word_copy);
                // destroy our original to avoid leak
                my_string_destroy(&word_copy);
            }
        }
    }
    fclose(fp);
    fp = NULL;

    if (generic_vector_get_size(dict_list) == 0) {
        printf("The Computer has 0 possibilities remaining.\n"
               "Try again with a different word length.\n");
        goto cleanup;
    }

    /* 3) Start the game working off the full dictionary */
    current_list = dict_list;
    dict_list    = NULL;  // ownership transferred

    /* 4) Build the initial key (all dashes) */
    key = my_string_init_default();
    if (!key) goto cleanup;
    for (int i = 0; i < word_length; i++) {
        my_string_push_back(key, '-');
    }

    /* 5) Prepare guessed_letters and the empty family_tree */
    guessed_letters = my_string_init_default();
    if (!guessed_letters) goto cleanup;

    family_tree = tree_init_default();
    if (!family_tree) {
        fprintf(stderr, "Could not initialize family tree.\n");
        goto cleanup;
    }

    /* 6) Main game loop */
    while (number_of_guesses > 0 &&
           strchr(my_string_c_str(key), '-') != NULL)
    {
        printf("\nYou have %d guesses left.\n", number_of_guesses);
        printf("Used Letters: %s\n",
               my_string_get_size(guessed_letters) > 0
                 ? my_string_c_str(guessed_letters)
                 : "(none)");
        printf("Word: %s\n", my_string_c_str(key));
        printf("Please guess a character: ");

        char guess;
        scanf(" %c", &guess);
        guess = tolower(guess);
        clear_keyboard_buffer();

        if (strchr(my_string_c_str(guessed_letters), guess)) {
            printf("You already guessed '%c'. Try another.\n", guess);
            continue;
        }
        my_string_push_back(guessed_letters, guess);

        /* Rebuild the family tree for this guess */
        tree_destroy(&family_tree);
        family_tree = tree_init_default();
        for (int i = 0; i < generic_vector_get_size(current_list); i++) {
            MY_STRING word    = generic_vector_at(current_list, i);
            MY_STRING new_key = my_string_init_default();
            get_word_key_value(key, new_key, word, guess);
            tree_add(family_tree, new_key, word);
            my_string_destroy(&new_key);
        }

        GENERIC_VECTOR largest = tree_largest_word_bin(family_tree);
        if (!largest || generic_vector_get_size(largest) == 0) {
            printf("No words matched. This shouldn't happen. Exiting.\n");
            if (largest) generic_vector_destroy(&largest);
            goto cleanup;
        }

        /* Destroy the old list (frees its MY_STRINGs) */
        generic_vector_destroy(&current_list);
        current_list = largest;

        /* Check if guess was correct */
        MY_STRING sample   = generic_vector_at(current_list, 0);
        MY_STRING next_key = my_string_init_default();
        get_word_key_value(key, next_key, sample, guess);

        Boolean correct = FALSE;
        for (int i = 0; i < word_length; i++) {
            if (my_string_c_str(key)[i] != my_string_c_str(next_key)[i]) {
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
        key = next_key;
    }

    /* 7) Endgame message */
    if (strchr(my_string_c_str(key), '-') == NULL) {
        printf("\nCongratulations! You guessed the word: %s\n",
               my_string_c_str(key));
    } else {
        MY_STRING answer = generic_vector_at(current_list, 0);
        printf("\nSorry, you lost. The word was: %s\n",
               my_string_c_str(answer));
    }

cleanup:
    if (fp)                fclose(fp);
    if (family_tree)       tree_destroy(&family_tree);
    if (guessed_letters)   my_string_destroy(&guessed_letters);
    if (key)               my_string_destroy(&key);
    if (temp_word)         my_string_destroy(&temp_word);
    if (current_list)      generic_vector_destroy(&current_list);
    if (dict_list)         generic_vector_destroy(&dict_list);
}
