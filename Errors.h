#ifndef _ERROR_H_
#define _ERROR_H_

#include "Tokenizer.h"

#include <string>

// ͬʱУ��token�����ͺ�����ֵ
void unExpectedToken(const Token& wrongToken, const Token& expectedToken);

#endif //_ERROR_H_
