#include "sfs_api.h"
#include "disk_emu.h"
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>


super_block sb; 
unsigned char freeblocks[BLOCK_NUM]; 
i_node inode_table[INODE_NUM];
directory root[INODE_NUM];  
fd_table_t fd_table[MAX_FILE]; 
int dirptr = 0; 

void zero_everything(){
	bzero(&sb, sizeof(super_block)); 
	bzero(&fd_table[0], sizeof(fd_table_t)*MAX_FILE);
	bzero(&inode_table[0], sizeof(i_node)*INODE_NUM);
	bzero(root, sizeof(directory)); 
	bzero(&freeblocks[0], sizeof(unsigned char)*BLOCK_NUM); 
}

void add_root_dir_inode(){
	inode_table[0].mode = 0x755; 
 	inode_table[0].link_cnt = 1; 
 	inode_table[0].uid = 0; 
 	inode_table[0].gid = 0;
 	inode_table[0].size = 0; 
 	inode_table[0].pointer[0] = 20;

}

//write the inode table into the disk 's blocks
void write_inode_table(){
	int i,j;
	i_node buf[7]; 
	for(i = 1; i<20; i++){
		for(j = 0; j<7; j++){
			if(i==19 && j==3)
				break; 
			buf[j] = inode_table[j*i];
		}
		write_blocks(i,1,buf);  
	}
}
//write the content of an inode to the disk 
void write_inode_by_index(int inode_num){
	int i,j = 0,  block_num = inode_num/7 ; 
	i_node buf[7]; 
	for(i = block_num*7 ; i<block_num*7+7; i++ ){
		if(i == 128)
			break; 
		buf[j] = inode_table[i];
		j++;  
	}
	write_blocks(block_num+1, 1, buf); 

}


//gets the data from an existing disk and put in the the assigned blocks 
void read_inode_table(){
	int i,j; 
	i_node buf[7]; 
	for(i = 1; i<20; i++){
			read_blocks(i,1,buf); 
			for (j = 0; j < 7; j++)
			{
				if(i==19 && j==3)
					break;
				inode_table[i*j] = buf[j]; 
			}
	}
}

//write the bitmap into the disk's blocks
void write_bitmap(){
	int i,j;
	int startblock = BLOCK_NUM-2-(BLOCK_NUM/(BLOCK_SIZE/sizeof(unsigned char)));  
	unsigned char buf[512]; 
	for (i = startblock ;i < BLOCK_NUM; i++){
		for (j = 0; i < 512; i++)
		{	
			if(i == BLOCK_NUM-1 && j == 369)
				break; 
			buf[j]=freeblocks[(i - startblock + 1)*j]; 
		}
		write_blocks(i, 1, buf); 
	}
}

//gets the data from an existing disk and put in the the assigned blocks 
void read_bitmap(){
	int i,j; 
	int startblock = BLOCK_NUM-1-(BLOCK_NUM/(BLOCK_SIZE/sizeof(unsigned char)));  
	unsigned char buf[512]; 
	for (i = startblock ; i < BLOCK_NUM-1; i++){
		read_blocks(i,1,buf);
		for (j = 0; i < 512; i++)
		{
			if(i == BLOCK_NUM-2 && j == 369) 
				break; 
			freeblocks[(i - startblock + 1)*j] = buf[j]; 
		}
		write_blocks(i, 1, buf); 
	}
}

int unvalid_name_check(char * name){
	int i, dot_index = -1, end_index = 0; 
	
	if( name == NULL )  return 1; 			// Check the validity of name as a string of at least 1 char  
	if( strlen(name) == 0 || strlen(name) > 20 ) return 1;	// and at most 20 chars
	
	for(i = 0; i<strlen(name); i++ ){	//check the length of the prefix, suffix and if there is multiple dots
		if(name[i] == '.'){
			if(dot_index != -1)			
				return 1; 
			dot_index = i; 
		}
		end_index++; 
	}
	if(dot_index > 17)
		return 1; 
	if(end_index - dot_index > 3)
		return 1; 
	return 0; 	 
}

void mksfs(int fresh) {
	int i; 
	if(fresh){
		printf("SFS init\n"); 
		init_fresh_disk(DISK_FILE,BLOCK_SIZE ,BLOCK_NUM);
		
		//everything is set to default value 0. 
 		zero_everything(); 

		/* superblock init and add */
		printf("superblock init\n");
		sb.magic = 17; 
		sb.block_size = BLOCK_SIZE;
		sb.fs_size = BLOCK_SIZE*BLOCK_NUM; 
		sb.inode_table_length = INODE_NUM; 
		sb.root_dir_node = 0; 

		write_blocks(0,1, &sb); 
		
		/* inode table init and add,root init and add */
		printf("inode table init\n");
		add_root_dir_inode();
		write_inode_table(); 
		write_blocks(20,1,&root);		//modify this 
		
		/* bitmap init and add*/
		printf("bitmap init\n");
		for(i = 0 ; i<20;i++)
			freeblocks[i] = 1; 	//superblock and inode table
		freeblocks[20] = 1;		//The root dir 
		
		/*blocks for storing the bitmap*/
		for(i=BLOCK_NUM-1; i>BLOCK_NUM-1-(BLOCK_NUM/(BLOCK_SIZE/sizeof(unsigned char))); i--)	
			freeblocks[i] = 1;
		write_bitmap();  


	}

	else{
		init_disk(DISK_FILE, BLOCK_SIZE, BLOCK_NUM); 
		read_blocks(0,1,&sb); 		//getting superblock
		read_blocks(20,1,&root);	//getting root
		read_inode_table();			//getting inode_table		
		read_bitmap(); 				//getting bitmap
	}

	return;
}

int sfs_getnextfilename(char *fname) {
	int i; 
	for (i = dirptr; i<100; i++){
		dirptr++; 
		if(root[i].inode !=0 ){
			fname = (char *) malloc(strlen(root[i].filename)); 	/* If there is file put its name */ 
			strcpy(fname, root[i].filename); 					/* in the buffer and return a positive*/
			return 1; 
		}
	}
	dirptr = 0; /* if no file has been found */ 
	return 0;	/* reset the start of search to 0 and return 0 */ 
}


int sfs_getfilesize(const char* path) {
	int i; 
	char* filename = strrchr(path, '\\');
	if(filename) filename++;
	else strcpy(filename, path);  
	for(i = 0; i<100; i++){
		if(root[i].inode != 0 && !strcmp(root[i].filename,filename)){	/* check if such file exist*/
			int inode_num = root[i].inode;								/* if yes then return size*/
			return inode_table[inode_num].size;	
		}
	}
	return -1;	//if the file does not exists return a negative
}

int sfs_fopen(char *name) {
	int i, check = 0, index;
	int inode_num = 0;  
	if(unvalid_name_check(name)) return 1 ; // checking for name validity (prefix of length <=16, one '.'' , suffix of length <= 3 ) 

	for(i = 0; i<100; i++){					//checking if the file exists
		if(!strcmp(root[i].filename,name)){
			inode_num = root[i].inode;
			break; 
		}
	}
								//if it indeed already exist
	if(inode_num){ 
		for(i = 0; i<100; i++){		//check whether it is already opened or not
			if( fd_table[i].inode_idx == inode_num){
					return i; 		/* if already opened then return its index */ 
			}
		}
								/* now check if there is an available */
								/* open file descriptor index */
		check = 0; 
		for(i = 0; i<100; i++){
			if(fd_table[i].inode_idx == 0){
				fd_table[i].inode_idx = inode_num; 
				fd_table[i].rd_write_ptr = inode_table[inode_num].size ;
				inode_table[inode_num].link_cnt = 1;
				check = 1;
				return i; 
				break; 
			}
		}
		//if open file descriptor is full, notify the user and return 
		if(!check){
			printf("Too much opened files, close one first\n"); 
			return 1; 
		}

	}
											//if the file is a new file
	else{
		for(i = 0; i<MAX_FILE ;i++ ){		/* check for available fd indexes */ 
			if(fd_table[i].inode_idx == 0){
				index = i;  
				check = 1; 
				break; 
			}
		}
		//if open file descriptor is full, notify the user and return 
		if(!check){			
			printf("Too much opened files, close one first\n"); 
			return 1; 
		}

		check = 0; 
		for(i = 0; i<128; i++){				/* if available then initialiaze inodetable and */  
			if(inode_table[i].mode == 0){			
				inode_table[i].mode 	= 255;
				inode_table[i].gid 		= 0;
				inode_table[i].uid 		= 0; 
				inode_table[i].link_cnt = 1; 
				inode_table[i].size 	= 0;
				//we do not modify the pointers to block;
				fd_table[index].inode_idx = i ; 
				fd_table[index].rd_write_ptr = 0;
				write_inode_table(); 
				check =1;  
				break; 
			}
		}						/* if no available inodes, return 1 */
		if(!check){			
			printf("Too much inode used, delete a file first\n"); 
			return 1; 
		}

		check =0; 					/* check for available index in root */
		for (i = 0; i <  100; i++){ 			/* if available then add to root */ 
			if(root[i].inode == 0){
				root[i].inode = fd_table[index].inode_idx; 
				strcpy(root[i].filename,name);
				check =1; 
				break;
			}
		}
		if(!check){						
			printf("No more indexes in root dir, delete a file first\n"); 
			return 1; 
		}
	}
	return 0;
}

int sfs_fclose(int fileID){
	int inode_num;

	if(fileID < 0 || fileID 100)	/* checking for valid input */
		return 1; 

	inode_num = fd_table[fileID].inode_idx; 
	if(!inode_num) return 1; 		/* if the selected fd entry is not open just return*/

	fd_table[fileID].inode_idx = 0; 	/* set fd table entry to default*/
	fd_table[fileID].rd_write_ptr =0;

	inode_table[inode_num].link_cnt = 0; /* set the number of link to the inode to 0*/

	//write_inode_by_index(inode_num); 
	return 0;
}

int sfs_fread(int fileID, char *buf, int length){
	int rw_ptr, block_num, data_ptr, inode_num, filled,block_ptr, added; 
	char temp[BLOCK_SIZE];
	int indirect[BLOCK_SIZE/sizeof(int)]; 

	if(fileID < 0 || fileID 100)	/* checking for valid input */
		return 1; 


	if(fd_table[fileID].inode_idx == 0 || 
		buf==NULL || length < 0 ){			/* checking if file is opened in fd_table, */
		 return 1 ; 						/*  and incorrect inputs */ 
	}

	inode_num = fd_table[fileID].inode_idx; 
	rw_ptr = fd_table[fileID].rd_write_ptr;  

	if(rw_ptr + length > inode_table[inode_num].size){			//Modify length if necessary	
		printf("The length is too long we will read until EOF\n");
		length = inode_table[inode_num].size - rw_ptr;   
	}

	block_num = rw_ptr/BLOCK_SIZE % 13;		/* getting the block we have to start at */
	data_ptr = rw_ptr % BLOCK_SIZE; 		/* getting the pointer to where to start in the block */
	filled = 0; 							/* get track of how much data (in bytes) have been read */
	
	while(length > 0 ){
		if(block_num > 139){
			printf("file is too long\n"); 
			return 1 ;
		}
		else if(block_num < 13){					/* if the block is directly pointed */ 
			block_ptr = inode_table[inode_num].pointer[block_num]; 
			read_blocks(block_ptr, 1, temp);
			added = (BLOCK_SIZE - data_ptr > length ? length : (BLOCK_SIZE - data_ptr) );  
			memcpy(buf+filled, temp + data_ptr, added);	//read into the buffer without erasing what was added before
			filled += added ; 
			length -= added ;
			if(added == length)
				data_ptr += length; 
			else 
				data_ptr = 0;  
			block_num++;  
		}
		else{								/* If the block is part of the undirectly pointed blocks */ 
			block_ptr = inode_table[inode_num].pointer[block_num]; //get the block of pointers
			read_blocks(block_ptr, 1, indirect);
			block_num -= 13; 				/* getting where the block is */ 
			block_ptr = indirect[block_num]; 
			read_blocks(block_ptr, 1, temp);
			added = (BLOCK_SIZE - data_ptr > length ? length : (BLOCK_SIZE - data_ptr) );  
			memcpy(buf+filled, temp + data_ptr, added);	//read into the buffer without erasing what was added before
			filled += added ; 
			length -= added ;
			if(added == length)
				data_ptr += length; 
			else 
				data_ptr = 0; 
			block_num++;  
		}	
	}
	return 0;
}


int sfs_fwrite(int fileID, const char *buf, int length){
	int rw_ptr, block_num, data_ptr, inode_num, filled,block_ptr, added; 
	int free_idx = -1; 
	int indirect_idx = -1;
	int i; 
	char temp[BLOCK_SIZE]; 
	int indirect[BLOCK_SIZE/sizeof(int)]; 

	if(fileID < 0 || fileID 100)	/* checking for valid input */
		return 1; 

	if(fd_table[fileID].inode_idx == 0 || 
		length < 0 ){			/* checking if file is opened in fd_table, */
		 return 1 ; 						/*  and incorrect inputs */ 
	}

	inode_num = fd_table[fileID].inode_idx; 
	rw_ptr = fd_table[fileID].rd_write_ptr;  

	if(rw_ptr + length > inode_table[inode_num].size){			//Modify length if necessary	
		printf("The length is too long we will write until EOF\n");
		length = inode_table[inode_num].size - rw_ptr;   
	}

	block_num = rw_ptr/BLOCK_SIZE % 13;		/* getting the block we have to start at */
	data_ptr = rw_ptr % BLOCK_SIZE; 		/* getting the pointer to where to start in the block */
	filled = 0; 							/* get track of how much data (in bytes) have been read */
	
	while(length > 13){
		if(block_num > 139){
			printf("file is too long\n"); 
			return 1 ;
		}
		else if(block_num < 13){
			if((block_ptr = inode_table[inode_num].pointer[block_num]) == 0){	//if need to write to a new block 
				for(i = 0; i<6000 ; i++){
					if(freeblocks[i] ==  0){
						free_idx == i ; 
						freeblocks[i] = 1;
						break;
					}
				}
				if(free_idx == -1){
					printf("No more blocks available (indirect)\n");
					return 1; 
				}
				block_ptr = free_idx; 
				inode_table[inode_num].pointer[block_num] = block_ptr; 
			}
			read_blocks(block_ptr, 1, temp);
			added = (BLOCK_SIZE - data_ptr > length ? length : (BLOCK_SIZE - data_ptr) );  
			memcpy(temp + data_ptr, buf+filled,  added);	//read into the buffer without erasing what was added before
			write_blocks(block_ptr, 1, buf); 
			filled += added ; 
			length -= added ;
			if(added == length)
				data_ptr += length; 
			else 
				data_ptr = 0; 
			block_num++;  
		}
		else{
			if((block_ptr = inode_table[inode_num].pointer[block_num]) == 0){	//if need to write to a new block 
				for(i = 0; i<6000 ; i++){
					if(freeblocks[i] ==  0){			//look for a new block for
						indirect_idx == i ; 			//block pointers\
						freeblocks[i] = 1; 
						break;
					}
				}
				if(indirect_idx == -1){
					printf("No more blocks available (for block of block_ptr )\n");
					return 1; 
				}
				inode_table[inode_num].pointer[block_num] = indirect_idx; 
				for(i = 0; i<6000 ; i++){
					if(freeblocks[i] ==  0){
						free_idx == i ; 
						freeblocks[i] = 1;
						break;
					}
				}
				if(indirect_idx == -1){
					printf("No more blocks available (for block_ptr in block)\n");
					return 1; 
				}
				read_blocks(indirect_idx, 1, indirect); 
				indirect[0] = free_idx; 
				write
			}
			else{

			}
		}




	}

	
	return 0;
}

int sfs_fseek(int fileID, int loc){

	if(fileID < 0 || fileID 100)	/* checking for valid input */
		return 1; 

	if(fd_table[fileID].inode_idx == 0 || loc < 0 || loc >= 71680)	//check if file is opened
		return 1 ; 													// if locationhas a legal value
	
	fd_table[fileID].rd_write_ptr = loc; 

	return 0;
}

int sfs_remove(char *file) {
	int file_index = -1, fd_index = -1, inode_idx ; 
	int i ; 
	int nulbuffer[BLOCK_SIZE/sizeof(int)]; 
	int temp[BLOCK_SIZE/sizeof(int)]; 
	memset(nulbuffer, 0 , BLOCK_SIZE); 

	if(unvalid_name_check(file)) return 1; 		//Check validity of name
	
	for(i = 0; i<100; i++){
		if(!strcmp(root[i].filename, file));	//look for the file in the root directory 
			file_index = i ;
			break; 
	}
	if(file_index == -1) return 1; 
    
	inode_idx = root[file_index].inode;

	for(i = 0; i<100 ; i++){					//Check if file is opened 
		if(fd_table[i].inode_idx == inode_idx)
			fd_index = i; 
	}

	if(fd_index != -1) sfs_fclose(fd_index); 	// If opened then close it 

	inode_table[inode_idx].mode= 0; 
	inode_table[inode_idx].uid = 0; 
	inode_table[inode_idx].gid = 0;
	inode_table[inode_idx].size = 0; 
	inode_table[inode_idx].link_cnt = 0; 
	for(i = 0; i<13; i++){
		if(inode_table[inode_idx].pointer[i] != 0){
			write_blocks(inode_table[inode_idx].pointer[i], 1, nulbuffer);
			freeblocks[inode_table[inode_idx].pointer[i]] = 0; 
			inode_table[inode_idx].pointer[i] = 0;  
		}
	}
	read_blocks(inode_table[inode_idx].pointer[13], 1, temp); 
	for(i = 0; i<sizeof(temp); i++){
		write_blocks(temp[i], 1, nulbuffer);
		freeblocks[temp[i]] = 0; 
	}
	inode_table[inode_idx].pointer[13] = 0; 

	root[file_index].inode = 0; 
	memset(root[file_index].filename, 0, 20*sizeof(char));

	return 0;
}
