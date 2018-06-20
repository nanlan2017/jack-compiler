#ifndef _ERROR_H_
#define _ERROR_H_

#include "Tokenizer.h"

#include <string>

// 同时校验token的类型和字面值
void unExpectedToken(const Token& wrongToken, const Token& expectedToken);

#endif //_ERROR_H_
