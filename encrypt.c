#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

void encrypt(char string[65],char hide[3][65],int select );
void decrypt(char hide[3][65],int select );
void make(char crypt[65],char original[64], char hide[3][65],int select );

int main()
{
    int i, slack=0;
    long int size=0;
    char c, keys[3][65];
    
  
    //keys derived from: GRC's Ultra High Security Password Generator (https://www.grc.com/passwords.htm)
    
    for(i=0;i<3;i++)
    {
        if(i==0)
        {
             strcpy(keys[i],"D95CB76299BFDD37DA4sD35B652919002BF577C1E6B2E497A3E0B80A1DF5B6376");
            
        }
        else if(i==1)
        {
            strcpy(keys[i],"<@(=}doI>!GhnBd!OsdZSSF*/;_E$-Ai(/$5/:+HUqk@za4]+hbM&_5K:_Vrg?P!");
        }
        else if(i==2)
        {
            strcpy(keys[i],"+rEMo&y:,v4I(p]*14A{!yWr{^cL6YHN@yj,08*#-DrBjk8[JzbJAD;G7=y9Q0s");

        }
    }
    
    FILE* ptr=fopen("text","r+");
    if(ptr==NULL)
    {
        printf("Couldnot open file!\n");
        return 1;
    } 
    
    //Making total number of character in file divisible by 64; encryption done on 64 character substrings of file;
    
    for(c=fgetc(ptr); c!=EOF; c=fgetc(ptr) )
    {
        size++;
    }  
    
    printf("Size= %ld\n",size);
    
    if( size%64!=0 )
    {
        int smaller=size%64;
        while(slack<64-smaller)
        {
            fputc('\0',ptr);
            slack++;
        }
    }
    //roll back pointer
    fseek(ptr, 0 , SEEK_SET);
    
    
    int index=0, select=0; //Change select by BMP colour detection
    char word[65]; //encrypt on a check of 64 (+1 NULL ) characters at a time; for full usage of key; increased strength
    
    // Encryption begins
    for(c=fgetc(ptr); c!=EOF; c=fgetc(ptr) )
    {
        word[index++]=c;
        
        if(index==64)
        {
            word[index]='\0'; //64th is NULL
            //printf("%s \t||\t\n",word);
            encrypt(word,keys,select); //select depends on the signature color in the BMP that determines which one of the three keys is to be used.
            
            index=0;
        }
    }
    fclose(ptr);
    
    decrypt(keys,select);
    
    
    
}
void encrypt(char string[65],char hide[3][65],int select )
{
    
    int i,upper_b=126,lower_b=32,rotate;
    char crypto[64]; //NULL appended
    
    FILE *into=fopen("cypto.txt","a");
    
    //printf("%s\n",hide[0]);
    
    rotate=upper_b-lower_b;
    for(i=0; i <65; i++)
    {
        
        crypto[i]=( ( (string[i]-lower_b) + (hide[select][i]-lower_b) )%rotate + lower_b);
        
    }
    
    fwrite(crypto, sizeof(crypto) , 1 , into);
    fclose(into);
    
}
void decrypt(char hide[3][65],int select )
{
    
    
    FILE *out=fopen("cypto.txt","r");
    
    FILE *actual=fopen("original.txt","a");
    
    if(out==NULL)
    {
        printf("Couldnot open Cryptic file!\n");
        return ;
    } 
    
    int index=0;
    char c,word[65], original[64];
    
    for(c=getc(out); c!=EOF; c=getc(out))
    {
        word[index++]=c;
        if( index==64 )
        {
            make(word,original,hide,select);
            index=0;
            
            //fputc('\n',out);
            fwrite(original, sizeof(original), 1, actual);
            
            
        }
    }
    
   fclose(out);
   
   fclose(actual);
    
   
}

void make(char crypt[65],char original[64], char hide[3][65],int select )
{
     int i,upper_b=126,lower_b=32,rotate;
     
     
     rotate=upper_b-lower_b;
    
    for(i=0; crypt[i]!='\0'; i++)
    {
            
            //frequency subtractor
            original[i]= ( (crypt[i]-lower_b) - (hide[select][i]-lower_b) );
            
            //fix overflow if any
            if(original[i]<0)
            {
                 original[i]= ( (crypt[i]-lower_b) + rotate - (hide[select][i]-lower_b) );
            }
            
            //make ASCII again 
            original[i]+= lower_b;
        
    }
    
    
}