#include "../source/taskable.h"
#include <iostream>

class cTest : public glib::Taskable
{
public:
	cTest(int _taskID) : 
	mTaskId(_taskID)
	{
	}

protected:
	int	mTaskId;
	void main(void)
	{
		using namespace std;
		
		for (int f=0;f<10;f++)
		{
			cout << "Hello from Task Id " << mTaskId ;
			cout << " (frame " << f << ")" << endl;
			sleep();
		}
	}
};


int main(void)
{
	using namespace glib;
	using namespace std;
	
	TSK_OpenModule();

	new cTest(1);
	new cTest(2);
	new cTest(3);
	
	for (int f=0; f<11; f++)
	{
		cout << "Task frame  " << f << endl;
		TSK_DoTasks();
	}

	return 0;

}

