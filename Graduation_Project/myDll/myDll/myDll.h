#ifndef _ENCRYPTBASE_H //定义_ENRYPTBASE_H宏，是为了防止头文件的重复引用
#define _ENCRYPTBASE_H

#ifdef __cplusplus //而这一部分就是告诉编译器，如果定义了__cplusplus(即如果是cpp文件， 
extern "C" { //因为cpp文件默认定义了该宏),则采用C语言方式进行编译
#endif


#ifdef DLL_EXPORTS  
#define DLL_EXPORTS __declspec(dllexport)   
#else  
#define DLL_EXPORTS __declspec(dllimport)   
#endif  

    //DLL_EXPORTS int Sum(int value1, int value2);

    //DLL_EXPORTS int Multiplication(int value1, int value2);
#ifdef __cplusplus
}
#endif


#endif // !_ENCRYPTBASE_H