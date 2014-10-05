#ifndef NS_ERROS_H
#define NS_ERROS_H

namespace ns {

enum class errors : int {
	NS_TIMEOUT = 1000,
	WILL_RECONNECT,
	BUFFER_LIMIT_REACHED
};

}

#endif
