// Author: Sam Ervolino
// Credit to Dr. Szumlanski for printTrieHelper and printTrie functions

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "TriePrediction.h"

// Functional prototypes of non required functions (not in header file).
TrieNode *createTrieNode(void);

void getMostFrequentWordHelper(TrieNode *root, char *buffer, char *mostFreq, int *frequency, int n);

void insertSubTrie(TrieNode *root, char w0[1024], char w1[1024]);

void insertString(TrieNode *root, char *str);

int prefixCountHelper(TrieNode *node, int count);

void printTrieHelper(TrieNode *root, char *buffer, int k);

void printTrie(TrieNode *root, int useSubtrieFormatting);

void processInput(char *filename, TrieNode *root);

void stripPunctuators(char *str, int *endSentence);

void textPrediction(TrieNode *root, char *w0, int n);

// Main function signature written by Dr. Szumlanski
int main(int argc, char **argv)
{
	TrieNode *root;

	root = buildTrie(argv[1]);

	processInput(argv[2], root);

	root = destroyTrie(root);

	return 0;
}

TrieNode *buildTrie(char *filename)
{
// We scan one at a time, and store each word into the sentence array.
// We know there can be no more than 30 words per sentence and
// no more than 1024 characters per word, so the array can hold
// one sentence at a time. j represents which word of the sentence
// we are processing, and endSentence is a flag telling us if there
// is a '!' , '?', or '.' that marks the end of the sentence.
	FILE *cfp;
	int j = 0, *endSentence = malloc(sizeof(int));
	char buffer[1026], sentence[30][1026];
	TrieNode *root = createTrieNode();
	fflush(stdout);

	if(endSentence == NULL)
	{
		printf("Malloc failed.\n");
		free(endSentence);
		return NULL;
	}

	if ((cfp = fopen(filename, "r")) == NULL)
	{
		printf("Failed to open corpus file.\n");
		cfp = NULL;
		return NULL;
	}

	while(fscanf(cfp, "%s", buffer) != EOF)
	{
// strip and check if we need to make room for another sentence after processing.
// after storing a copy, we insert it into the main trie and into the
// appropriate subtrie if necessary. then clear the buffer and go again.
		*endSentence = 0;
		stripPunctuators(buffer, endSentence);
		strcpy(sentence[j], buffer);
		insertString(root, buffer);

		if(j != 0)
			insertSubTrie(root, sentence[j-1], sentence[j]);

		memset(buffer, 0, sizeof(buffer));
		j++;

		if(*endSentence == 1)
		{
			j = 0;
			memset(sentence, 0, sizeof(sentence[0][0]) * 30 * 1024);
		}
	}

	fclose(cfp);
	cfp = NULL;
	free(endSentence);

	return root;
}

int containsWord(TrieNode *root, char *str)
{
	int length, i, index;
	TrieNode *temp;

	if(root == NULL || str == NULL)
		return 0;

	else
	{
		temp = root;
		length  = strlen(str);

		for(i = 0; i < length; i++)
		{
			index = str[i] - 'a';

			if(temp->children[index] != NULL)
				temp = temp->children[index];
			else
				return 0;
		}

		if(temp->count >= 1)
			return 1;
		else
			return 0;
	}

}

TrieNode *createTrieNode(void)
{
	int i;
	TrieNode *n = malloc(sizeof(TrieNode));

	if(n == NULL)
	{
		printf("Malloc failed\n");
		free(n);
		return NULL;
	}

	for(i = 0; i < 26; i++)
		n->children[i] = NULL;

	n->subtrie = NULL;

	n->count = 0;

	return n;
}

TrieNode *destroyTrie(TrieNode *root)
{
	int i;
	
	if(root == NULL)
		return NULL;
// First check for subtries, and destroy if there is one. then work from the bottom
// up on all child pointers until we come back to the root.
	else
	{
		if(root->subtrie != NULL)
			root->subtrie = destroyTrie(root->subtrie);

		for(i = 0; i < 26; i++)
		{
			if(root->children[i] != NULL)
				root->children[i] = destroyTrie(root->children[i]);

			else
				continue;
		}

		free(root);
	}

	return NULL;
}

// frequency keeps track of the occurrences of the most frequent word in the trie. 
// str stores the most frequent word if multiple words are tied for highest frequency,
// then the one that comes first in alphabetic order is the one stored into str.
// buffer stores the most frequent word one letter at a time, to be transferred
// into str at the end.
void getMostFrequentWord(TrieNode *root, char *str)
{
	int i, *frequency = malloc(sizeof(int));
	char *buffer = malloc(sizeof(char) * 1026);

	if(root == NULL)
	{
		memset(str, 0, sizeof(str));
		strcpy(str, "");
	}

	else
	{
		strcpy(buffer, "");
		*frequency = 0;
		getMostFrequentWordHelper(root, buffer, str, frequency, 0);

		if(*frequency == 0)
		{
			memset(str, 0, sizeof(str));
			strcpy(str, "");
		}
	}
	free(frequency);
	free(buffer);
}

void getMostFrequentWordHelper(TrieNode *root, char *buffer, char *str, int *frequency, int n)
{
	int i;
	char index;

	if(root == NULL)
		return;


	else if(root->count > *frequency)
	{
		memset(str, 0, sizeof(str));
		strcpy(str, buffer);
		*frequency = root->count;
	}

	buffer[n+1] = '\0';

	for(i = 0; i < 26; i++)
	{
		if(root->children[i] != NULL)
		{
			buffer[n] = i + 'a';
			getMostFrequentWordHelper(root->children[i], buffer, str, frequency, n+1);
		}
	}
	buffer[n] = '\0';
}

TrieNode *getNode(TrieNode *root, char *str)
{
	TrieNode *temp;
	int i, index, length;

	if(root == NULL || str == NULL)
		return NULL;

	//printf("%s\n", str);

	temp = root;
	length = strlen(str);

	//printf("\t%d\n", length);

// Check and make sure the nodes that point to the string are even initialized as we go.
// return NULL if not initialized because it is therefor not in the trie.
// If it is initialized but there is no count, then also return NULL.
// Otherwise return the terminal node.
	for(i = 0; i < length; i++)
	{
		index = str[i] - 'a';

		if(temp->children[index] == NULL)
			return NULL;

		temp = temp->children[index];
	}

	if(temp->count >= 1)
		return temp;
	else
		return NULL;
}

void insertSubTrie(TrieNode *root, char w0[1024], char w1[1024])
{
	int length = strlen(w0), index, i;
	TrieNode *temp = root;

// The for loop traverses the trie to the string that will be the root of the subtrie
	for(i = 0; i < length; i++)
	{
		index = w0[i] - 'a';

		temp = temp->children[index];
	}

	if(temp->subtrie == NULL)
		temp->subtrie = createTrieNode();

	insertString(temp->subtrie, w1);
}

void insertString(TrieNode *root, char *str)
{
	int length = strlen(str), index, i;
	TrieNode *temp;

	if(root == NULL)
		return;
	if(str == NULL)
		return;

	temp = root;

// Convert each letter to index 0-25, and if the corresponding child node is equal to NULL,
// We go ahead and initialize it.
// Finally we increment the count once we are at the terminal node.
	for(i = 0; i < length; i++)
	{
		index = str[i] - 'a';

		if(temp->children[index] == NULL)
			temp->children[index] = createTrieNode();

		temp = temp->children[index];
	}

	temp->count++;
}

// This function uses temp to traverse through the trie to the terminal node of the prefix,
// if it exists. if it does, we implement prefixCountHelper. 
int prefixCount(TrieNode *root, char *str)
{
	TrieNode *temp;
	int i, index, length, count = 0;
	if(root == NULL || str == NULL)
		return 0;
	else
	{
		temp = root;
		length = strlen(str);

		for(i = 0; i < length; i++)
		{
			index = str[i] - 'a';

			if(temp->children[index] == NULL)
				return 0;
			else
				temp = temp->children[index];
		}

		count = prefixCountHelper(temp, count);

		return count;
	}
}

// This helper function recursively travels through the trie.
// If the current node has a count, we add that to our final 
// count. Otherwise we traverse until there are no more 
// initialized nodes to check.
int prefixCountHelper(TrieNode *node, int count)
{
	int i;

	if(node->count > 0)
		count += node->count;

	for(i = 0; i < 26; i++)
	{
		if(node->children[i] != NULL)
			count = prefixCountHelper(node->children[i], count);

		else
			continue;
	}

	return count;
}

// Helper function called by printTrie()
void printTrieHelper(TrieNode *root, char *buffer, int k)
{
	int i;

	if (root == NULL)
		return;

	if (root->count > 0)
		printf("%s (%d)\n", buffer, root->count);

	buffer[k + 1] = '\0';

	for (i = 0; i < 26; i++)
	{
		buffer[k] = 'a' + i;

		printTrieHelper(root->children[i], buffer, k + 1);
	}

	buffer[k] = '\0';
}

// If printing a subtrie, the second parameter should be 1; otherwise, if
// printing the main trie, the second parameter should be 0.
void printTrie(TrieNode *root, int useSubtrieFormatting)
{
	char buffer[1026];

	if (useSubtrieFormatting)
	{
		strcpy(buffer, "- ");
		printTrieHelper(root, buffer, 2);
	}
	else
	{
		strcpy(buffer, "");
		printTrieHelper(root, buffer, 0);
	}
}

// This function will read commands from the input file and call the appropriate function
// to execute it.
void processInput(char *filename, TrieNode *root)
{
	FILE *ifp;
	TrieNode *temp;
	char buffer[1026];
	int length, i, index, flag, n;

	if((ifp = fopen(filename, "r")) == NULL)
	{
		printf("Failed to open input file.\n");
		ifp = NULL;
	}
	else
	{
		while(fscanf(ifp, "%s", buffer) != EOF)
		{
			flag = 0;
// Check the first character to see if it's one of the input commands
			if(buffer[0] == '!')
			{
				printTrie(root, 0);
				memset(buffer, 0, sizeof(buffer));
			}

			else if(buffer[0] == '@')
			{
				memset(buffer, 0, sizeof(buffer));
				fscanf(ifp, "%s", buffer);
				fscanf(ifp, "%d", &n);
				printf("%s", buffer);

				textPrediction(root, buffer, n);

				memset(buffer, 0, sizeof(buffer));

				printf("\n");
			}

			else
			{
				printf("%s\n", buffer);
				length = strlen(buffer);
				temp = root;

// Check if the string is in the trie
				for(i = 0; i < length; i++)
				{
					if(islower(buffer[i]) == 0)
						index = tolower(buffer[i]) - 'a';
					else
						index = buffer[i] - 'a';

					if(temp->children[index] == NULL)
					{
						printf("(INVALID STRING)\n");
						memset(buffer, 0, sizeof(buffer));
						flag = 1;
						break;
					}
					 
					else
						temp = temp->children[index];
				
				}
				if(flag == 1)
					continue;
				
				else if(temp->count == 0)
					printf("(INVALID STRING)\n");

				else if(temp->subtrie == NULL)
					printf("(EMPTY)\n");

				else
					printTrie(temp->subtrie, 1);
				
				memset(buffer, 0, sizeof(buffer));
			}
		}
// Close files and ensure there are no memory leaks if main is ever repurposed
		fclose(ifp);
		ifp = NULL;
	}
}

// Create a new string of the same size, and only copy characters in lowercase into it.
// Then empty the original string to store the result and free the temp string.
// endSentence is a flag that will tell the build function if we need to make room for a new sentence.
void stripPunctuators(char *str, int *endSentence)
{
	int i, j = 0, length = strlen(str);
	char *new_str = malloc(sizeof(char) * length + 1);

	if(new_str == NULL)
	{
		printf("Malloc failed.\n");
		free(new_str);
		return;
	}

	for(i = 0; i < length; i++)
	{
		if(str[i] == '!' || str[i] == '?' || str[i] == '.')
			*endSentence = 1;

		if(isalpha(str[i]) == 0)
			continue;

		else if(islower(str[i]) == 0)
		{
			new_str[j] = (char) tolower(str[i]);
			j++;
		}
		else
		{
			new_str[j] = str[i];
			j++;
		}

	}

	new_str[j] = '\0';

	memset(str, 0, sizeof(str));
	strcpy(str, new_str);
	free(new_str);
}

void textPrediction(TrieNode *root, char w0[1026], int n)
{
	int i, length, index;
	char w1[1026];
	TrieNode *node;

	stripPunctuators(w0, 0);
	node = getNode(root, w0);

	if(n == 0)
		return;

	else if(node == NULL)
		return;

	else if(node->subtrie == NULL)
		return;

	else
	{
		getMostFrequentWord(node->subtrie, w1);
		printf(" %s", w1);
		textPrediction(root, w1, n-1);
		memset(w1, 0, sizeof(w1));
	}
}

double difficultyRating(void)
{
	return 4.0;
}

double hoursSpent(void)
{
	return 45.5;
}
