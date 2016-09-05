#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "bmp.h"

void colourdetect(char infile[50], int sequence[3000]);
void encrypt(char hide[3][65],int sequence[3000] );
void decrypt(char hide[3][65],int sequence[3000]);
void make(char crypt[65],char original[64], char hide[3][65],int select );
void destroy(char word[65],char scramble[64], char hide[3][65], int select);

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
        int smaller=size%(64-1);
        while(slack<(64-1)-smaller)
        {
            fputc('\0',ptr);
            slack++;
        }
        fputc(EOF,ptr);
    }
    //roll back pointer
    fseek(ptr, 0 , SEEK_SET);
    
    
    int sequence[3000];
    
    char sector[50];
    printf("Enter Image file name: ");
    scanf("%s",sector);
    
    
    colourdetect(sector,sequence);
    
    encrypt(keys,sequence);
    
    decrypt(keys,sequence);
    
    
    
}
void colourdetect(char infile[50], int sequence[3000])
{
   
    int length=0,index=0;
    
    for(int i=0; i<3000; i++)
    {
        sequence[i]=-1;
    }
    
    //strcpy(infile,"clue.bmp");
    
    FILE* inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        printf("Could not Image %s.\n", infile);
        return ;
    }
    
   
    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 || bi.biBitCount != 24 || bi.biCompression != 0)
    {
        
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return ;
    }
   
    // determine padding for scanlines
    int padding =  (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {
        // iterate over pixels in scanline
        for (int j = 0; j < bi.biWidth; j++)
        {
           
            RGBTRIPLE triple;
            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);
            
            //65 Value is assumed, ensure proper value
            
            if(length%151==0) //assume after entering 64 characters==150 triplets, we need to check the 151st RBG value, including 0;
            {
                if( (triple.rgbtRed>triple.rgbtBlue) && (triple.rgbtRed>triple.rgbtGreen) )
                {
                    sequence[index]=0; //0 for Red highest
                    index++;
                    
                }
                else if( (triple.rgbtBlue>triple.rgbtRed) && (triple.rgbtBlue>triple.rgbtGreen) )
                {
                    sequence[index]=1; //1 for Blue highest
                    index++;
                    
                }
                else
                {
                    sequence[index]=2; //2 for Green highest
                    index++;
                }
                
            }
            length++;
            
            
        }

        // skip over padding, if any
        fseek(inptr, padding, SEEK_CUR);

    }

    // close infile
    fclose(inptr);
    
/*    
    //check detected RBGs
    for(int i=0; i<3000; i++)
    {
        printf("%d = < %d >\n", i, sequence[i]);
    }

*/    
    

    
}
void encrypt(char hide[3][65],int sequence[3000])
{
    int index=0,select=0,move=0; //Change select by BMP colour detection
    char c,word[65], scramble[64]; //encrypt on a check of 64 (+1 NULL ) characters at a time; for full usage of key; increased strength
    
    FILE *ptr=fopen("text","r");
    
    FILE *into=fopen("cypto.txt","a");
    
    if(ptr==NULL)
    {
        printf("Couldnot open text file!\n");
        return ;
    } 
   
    
    // Encryption begins
    for(c=fgetc(ptr); c!=EOF; c=fgetc(ptr) )
    {
        word[index++]=c;
        
        if(index==64)
        {
            word[index]='\0'; //64th is NULL
            
            select=sequence[move];
            move++;

            destroy(word,scramble,hide,select); //select depends on the signature color in the BMP that determines which one of the three keys is to be used.
            fwrite(scramble, sizeof(scramble) , 1 , into);
            
             index=0;
        }
    }
    fclose(ptr);

    fclose(into);
    
}
void destroy(char word[65],char scramble[64], char hide[3][65], int select)
{
    int i,upper_b=126,lower_b=32,rotate;
    
    rotate=upper_b-lower_b;
    
    for(i=0; i <65; i++)
    {
        //frequency adder
        scramble[i]=( ( (word[i]-lower_b) + (hide[select][i]-lower_b) )%rotate + lower_b);
        
    }
    
}
void decrypt(char hide[3][65],int sequence[3000])
{
    
    
    FILE *out=fopen("cypto.txt","r");
    
    FILE *actual=fopen("original.txt","a");
    
    if(out==NULL)
    {
        printf("Couldnot open Cryptic file!\n");
        return ;
    } 
    
    int index=0,select=0,move=0;
    char c,word[65], original[64];
    
    for(c=getc(out); c!=EOF; c=getc(out))
    {
        word[index++]=c;
        if( index==64 )
        {
            select=sequence[move];
            move++;
            
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