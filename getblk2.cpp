#include<iostream>
#include<string.h>
#include<thread>
#include <stdlib.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include<unistd.h>
#include<string>
#define HASH(n) (n%4)



enum status_code { free_ = 1, locked, unlocked, valid, invalid, delay_write,old };
std::string codes[] = {"free", "locked", "unlocked", "valid", "invalid", "delayed write","old"};

class Buff_node
{
public:
	int blk_no;
	int device_no;
	int data;
	status_code  status;
	Buff_node *fowd_free_list;
	Buff_node *prev_free_list;
	Buff_node *fowd_hash_list;
	Buff_node *prev_hash_list;
	int buff_no;

	Buff_node() {
		blk_no = 0;
		device_no = 1;
		status = free_;
		fowd_free_list = nullptr;
		prev_free_list = nullptr;
		fowd_hash_list = nullptr;
		prev_hash_list = nullptr;
		buff_no = 0;
		data = 0;

	}
	Buff_node(int n) {
		blk_no = n;
		device_no = 1;
		status = free_;
		fowd_free_list = nullptr;
		prev_free_list = nullptr;
		fowd_hash_list = nullptr;
		prev_hash_list = nullptr;
		buff_no = 0;
		data = 0;

	}


};

Buff_node *hashque[4]{nullptr};

Buff_node *fhead = nullptr;







////////////////////////////////////////////////////////////////////////
// 					create initial buffers
////////////////////////////////////////////////////////////////////////
void CreatePool(int n) {
	Buff_node *temp = nullptr;
	for (int i = 0; i < n; ++i) {
		Buff_node *newBuff = new Buff_node();
		newBuff->buff_no = i + 1;

		if (fhead == nullptr) {
			fhead = newBuff;
			newBuff->fowd_free_list = fhead;
			newBuff->prev_free_list = newBuff;

			temp = newBuff;
		}
		else {
			temp->fowd_free_list = newBuff;
			newBuff->prev_free_list = temp;
			newBuff->fowd_free_list = fhead;
			fhead->prev_free_list = newBuff;
			temp = temp->fowd_free_list;
		}
	}

}



////////////////////////////////////////////////////////////////////////
//					print free list
////////////////////////////////////////////////////////////////////////
void printFreeList() {
	Buff_node *head = fhead;
	if (head == nullptr) {
		std::cout << "free list is empty" << std::endl;
		return;
	}
	else {
		Buff_node *temp = head;
		std::cout << "free list: ";


		while (temp->fowd_free_list != head) {
			std::cout << "["<< temp->buff_no<<":" <<temp->blk_no<<":"<<  codes[(temp->status) -1] <<"], ";
			temp = temp->fowd_free_list;

		}
		std::cout << "["  << temp->buff_no<<":" <<temp->blk_no <<":"<<  codes[(temp->status) -1] << "] " << std::endl;
	}
}




////////////////////////////////////////////////////////////////////////
//					print hash queues
////////////////////////////////////////////////////////////////////////
void printHashQ()  {

	for (int i = 0; i < 4; ++i) {
		if (hashque[i] == nullptr) {
			std::cout << "hashque " << i + 1 << ": empty." << std::endl;
		}
		else {
			std::cout << "hashque" << i + 1 << ": ";
			Buff_node* temp = hashque[i];
			while (temp->fowd_hash_list != nullptr) {

				std::cout << "[" << temp->buff_no <<":"<<temp->blk_no<< ":"<<  codes[(temp->status) -1] <<"], ";
				temp = temp->fowd_hash_list;
			}
			std::cout << "[" << temp->buff_no <<":"<<temp->blk_no<<":"<<  codes[(temp->status) -1]<< "] " << std::endl;


		}
	}
}




////////////////////////////////////////////////////////////////////////
//					add buffer in hash queue
////////////////////////////////////////////////////////////////////////
void addInHash(Buff_node **bp) {
	Buff_node *temp = (*bp);
	//std::cout<<"adding"<<bno<<" in hashq"<<std::endl;
	int hash_no = HASH(temp->blk_no);
	//std::cout<<"hash_no = "<<hash_no<<std::endl;
	//Buff_node *temp = nullptr;
	Buff_node **hashQ = &hashque[hash_no];
	

	if (*hashQ == nullptr) {

		*hashQ = temp;
		(*hashQ)->fowd_hash_list = nullptr;
		(*hashQ)->prev_hash_list = nullptr;


	}
	else {

		temp->fowd_hash_list = (*hashQ);
		temp->prev_hash_list = nullptr;
		(*hashQ)->prev_hash_list = temp;
		(*hashQ) = temp;

	}

	/*temp = fhead->fowd_free_list;
	fhead->prev_free_list->fowd_free_list = temp;
	temp->prev_free_list = fhead->prev_free_list;
	fhead = temp;*/



}




////////////////////////////////////////////////////////////////////////
//					check buffer in hash 
////////////////////////////////////////////////////////////////////////
Buff_node* inHash(int dev_no, int bno) {
	int hash_no = HASH(bno);
	//std::cout<<hash_no<<std::endl;
	Buff_node *temp = hashque[hash_no];
	if (temp == nullptr)
		return 0;

	while (temp->fowd_hash_list != nullptr) {
		//std::cout<<"in while loop"<<std::endl;
		if (temp->device_no == dev_no && temp->blk_no == bno) {
			//std::cout<<"in while if loop"<<std::endl;
			return temp;
		}
		temp = temp->fowd_hash_list;


	}
	if (temp->device_no == dev_no && temp->blk_no == bno) {
		//std::cout<<"outsise while loop"<<std::endl;
		return temp;
	}
	return 0;
}




/////////////////////////////////////////////////////////////////////////
//					insert buffer at head of free list
/////////////////////////////////////////////////////////////////////////
void insert_at_head(Buff_node** buff) {
	(*buff)->status = unlocked;
	Buff_node* temp = nullptr;
	if (fhead == nullptr) {
		fhead = *buff;
		fhead->fowd_free_list = fhead;
		fhead->prev_free_list = fhead;
	}
	else {
		temp = fhead->prev_free_list;
		temp->fowd_free_list = (*buff);
		(*buff)->prev_free_list = temp;
		(*buff)->fowd_free_list = fhead;
		fhead->prev_free_list = (*buff);
		fhead = (*buff);
	}
	

}

////////////////////////////////////////////////////////////////////////
//					remove from hash q
////////////////////////////////////////////////////////////////////

void removeFromHashQ( Buff_node **buff){
	int hash_no = HASH((*buff)->blk_no);
	//std::cout<<hash_no<<std::endl;
	Buff_node **head = &hashque[hash_no];
	//std::cout<<"block no. of gead: "<<head->blk_no;
	Buff_node *temp{nullptr};
	if((*head) == (*buff) ){
		(*head) = (*buff)->fowd_hash_list;
	}
	else if ((*buff)->fowd_hash_list == nullptr){
		(*buff)->prev_hash_list->fowd_hash_list = nullptr;
	}else{
		temp = (*buff)->prev_hash_list;
		temp->fowd_hash_list = (*buff)->fowd_hash_list;

	}

}



/////////////////////////////////////////////////////////////////////////
// 						insert buffer at tail
/////////////////////////////////////////////////////////////////////////
void insert_at_tail(Buff_node **buff){
	//(*buff)->status = unlocked;
	Buff_node* temp = nullptr;
	if(fhead == nullptr){
		fhead = *buff;
		fhead->fowd_free_list = fhead;
		fhead->prev_free_list = fhead;
	}else{temp = fhead->prev_free_list;
		temp->fowd_free_list = (*buff);
		(*buff)->prev_free_list = temp;
		(*buff)->fowd_free_list = fhead;
		fhead->prev_free_list = (*buff);

	}
	std::cout<<"status :"<<(*buff)->status<<std::endl;
}





/////////////////////////////////////////////////////////////////////////
//					remove buffer
/////////////////////////////////////////////////////////////////////////

void removeBufferFromTheList(Buff_node** bp){
	if((*bp) == fhead){
		if(fhead->fowd_free_list == fhead){
			fhead = nullptr;
		}else{
		Buff_node* temp = (*bp)->prev_free_list;
		temp->fowd_free_list = (*bp)->fowd_free_list;
		(*bp)->fowd_free_list->prev_free_list = temp;
		fhead = temp->fowd_free_list;
		}
	}else{
		Buff_node* temp = (*bp)->prev_free_list;
		temp->fowd_free_list = (*bp)->fowd_free_list;
		(*bp)->fowd_free_list->prev_free_list = temp;
		//(*bp)->fowd_free_list = (*bp)->prev_free_list = nullptr;
	}
}


void **brelse(Buff_node **buff){
    std::cout<<"waking up all processes..."<<std::endl;
    if((*buff)->status == old && (*buff)->status == valid ){
        std::cout<<"inserting at head of free list"<<std::endl;
        (*buff)->status = unlocked;
        insert_at_head(buff);
    }else{
        (*buff)->status = invalid;
        std::cout<<"inserting at tail of free list"<<std::endl;
        insert_at_tail(buff);
    }

}

void writeAsync(Buff_node **buf){
	std::cout<<"writing buffer to disk...\n ";
	Buff_node *temp = (*buf);
	removeFromHashQ(buf);
	removeBufferFromTheList(buf);
	temp->status = old;
	insert_at_head(&temp);
}


/////////////////////////////////////////////////////////////////////////
//						get block algorithm
/////////////////////////////////////////////////////////////////////////
Buff_node*  getblk(int dev_no,int bno){
	Buff_node* bp{nullptr};
	while(1){
		bp = inHash(dev_no,bno);
		if( bp != 0){
			if (bp->status == locked ){
				std::cout<<"buffer is in hash queue & busy\n";
				std::cout<<"sleep\n"<<std::endl;
				break;
			}
			else{
				std::cout<<"buffer is in hash queue and free\n";
				bp->status = locked;
				
				removeBufferFromTheList(&bp);
				
				return bp;
			}
		}
		else{
			bp =fhead;
			if(fhead == nullptr){
				std::cout<<"free list empty\nprocesss to sleep.\n"<<std::endl;
				break;
			}
			else{
				if(bp->status == delay_write){
					// std::cout<<"writing buffer to disk...\n ";
					// removeBufferFromTheList(&fhead);
					writeAsync(&bp);
					continue;
				}else if(bp->status == invalid){
					std::cout<<"allocating buffer from else if\n";
					//bp = fhead;
					removeFromHashQ(&bp);
					//printHashQ();
					bp->status = locked;
					bp->blk_no = bno;
					bp->device_no = dev_no;
					
					removeBufferFromTheList(&bp);
					
					addInHash(&bp);
					return bp;
				}
				
				else{
					std::cout<<"allocating buffer\n";
					//bp = fhead;
					bp->status = locked;
					bp->blk_no = bno;
					bp->device_no = dev_no;
					
					removeBufferFromTheList(&bp);
					addInHash(&bp);
					return bp;
				}
			}
		}
	}
}

struct mesg_buffer { 
    long mesg_type;
	int pid = getpid();
	int choice;
    int dev_no;
    int blk_no;
} message;  




int main() {

	// List l1;
	
    int block_number{0};
    int delete_b_no{0};
	CreatePool(10);
	printFreeList();

    key_t key; 
    int msgid;
	key = ftok("progfile", 65); 
    msgid = msgget(key, 0666 | IPC_CREAT); 
    while(1){
		std::cout<<"=======================================================================================================\n";
        msgrcv(msgid, &message, sizeof(message), 1, 0);
        switch(message.choice){
            case 1:{
                std::cout<<"Process id "<<message.pid<<" require the block no "<<message.blk_no<<std::endl;
                getblk(message.dev_no,message.blk_no);
            }
                break;
            case 2:{
                Buff_node* temp = inHash(message.dev_no,message.blk_no);
                brelse(&temp);
                break;
            }
            case 3:
                break;

            default:
                std::cout<<"\nYou entered the wrong choice \n";
                break;

        } 
        
        
        // printf("Sum of the numbers is : %d \n",sum); 
        printFreeList();
        printHashQ();
    }
        // to destroy the message queue 
        msgctl(msgid, IPC_RMID, NULL); 
    



	
	return 0;
	
	
}
