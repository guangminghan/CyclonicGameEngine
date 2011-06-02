#ifndef   __PAUSEME_H  
#define   __PAUSEME_H
class PAUSEME
{  
  public:  
    PAUSEME()
	{  
        std::atexit(&pause);  
    }  
    static   void   pause()
	{   
		system("pause");   
	}  
};  
#endif