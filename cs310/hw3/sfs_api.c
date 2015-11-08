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

void write_inode_table(){
	int i,j;
	i_node buf[7]; 
	for(i = 1; i<=19; i++){
		for(j = 0; j<7; j++){
			if(i==19 && j==3)
				break; 
			buf[j] = inode_table[j*i];
		}
		write_blocks(i,1,buf);  
	}
}

void write_bitmap(){
	int i,j;
	int startblock = BLOCK_NUM-1-(BLOCK_NUM/(BLOCK_SIZE/sizeof(unsigned char)));  
	unsigned char buf[512]; 
	for (i = startblock ;
			 i < BLOCK_NUM-1; i++){
		for (j = 0; i < 512; i++)
		{
			buf[j]=freeblocks[(i - startblock + 1)*j]; 
		}
		write_blocks(i, 1, buf); 
	}
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
		
		/* inode table init and add,rootdir init and add */
		printf("inode table init\n");
		add_root_dir_inode();
		write_inode_table(); 
		write_blocks(20,1,&root); 
		
		/* bitmap init and add*/
		printf("bitmap init\n");
		for(i = 0 ; i<20;i++)
			freeblocks[i] = 1; 	//superblock and inode table
		freeblocks[20] = 1;		//The root dir 
		
		//blocks for storing the bitmap
		for(i=BLOCK_NUM-1; i>BLOCK_NUM-1-(BLOCK_NUM/(BLOCK_SIZE/sizeof(unsigned char))); i--)	
			freeblocks[i] = 1;
		write_bitmap();  


	}

	else{
		init_disk(DISK_FILE, BLOCK_SIZE, BLOCK_NUM); 
		read_blocks(0,1,&sb);

	}

	return;
}

int sfs_getnextfilename(char *fname) {

	//Implement sfs_getnextfilename here	
	return 0;
}


int sfs_getfilesize(const char* path) {

	//Implement sfs_getfilesize here	
	return 0;
}

int sfs_fopen(char *name) {

	//Implement sfs_fopen here	
	return 0;
}

int sfs_fclose(int fileID){

	//Implement sfs_fclose here	
	return 0;
}

int sfs_fread(int fileID, char *buf, int length){

	//Implement sfs_fread here	
	return 0;
}

int sfs_fwrite(int fileID, const char *buf, int length){

	//Implement sfs_fwrite here	
	return 0;
}

int sfs_fseek(int fileID, int loc){

	//Implement sfs_fseek here	
	return 0;
}

int sfs_remove(char *file) {

	//Implement sfs_remove here	
	return 0;
}
