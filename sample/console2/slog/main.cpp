#include <string.h>
#include <slog.h>
#include <systemcall_impl.h>

int main(int argc, char* argv[]) 
{

	char char_arg[32];
	strcpy(char_arg, "test string");
	int int_arg = 69;

	//slog library initialization
	slog_init("example", 0, 3, 0);

	slog(0, SLOG_LIVE, "Test message with level 0");
	slog(1, SLOG_WARN, "Warn message with level 1");
	slog(2, SLOG_INFO, "Info message with level 2");

	slog(3, SLOG_LIVE, "Test message with level 3");
	slog(0, SLOG_DEBUG, "Debug message with char argument: %s", char_arg);
	slog(0, SLOG_ERROR, "Error message with int argument: %d", int_arg);

	//파일로 로그 기록 전환
	SlogConfig slgCfg;
	slog_config_get(&slgCfg);
	slgCfg.nToFile = 1;
	slog_config_set(&slgCfg);

	slog(0, SLOG_DEBUG, "Debug message in the file with int argument: %d", int_arg);

    return 0;
}