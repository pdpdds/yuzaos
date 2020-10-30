#include <string.h>
#include <slog.h>
#include <systemcall_impl.h>

int main(int argc, char* argv[]) 
{
	/* Used variables */
	char char_arg[32];
	strcpy(char_arg, "test string");
	int int_arg = 69;

	/* Greet users */
	//greet();

	/* Initialize slog with default parameters */
	slog_init("example", 0, 3, 0);

	/* Log and print something with level 0 */
	slog(0, SLOG_LIVE, "Test message with level 0");

	/* Log and print something with level 1 */
	slog(1, SLOG_WARN, "Warn message with level 1");

	/* Log and print something with level 2 */
	slog(2, SLOG_INFO, "Info message with level 2");

	/* Log and print something with level 3 */
	slog(3, SLOG_LIVE, "Test message with level 3");

	/* Log and print something with char argument */
	slog(0, SLOG_DEBUG, "Debug message with char argument: %s", char_arg);

	/* Log and print something with int argument */
	slog(0, SLOG_ERROR, "Error message with int argument: %d", int_arg);

	/* Test log with higher level than log max value
	* This will never be printed while log level argument is higher than max log level */
	slog(4, SLOG_NONE, "[LIVE] Test log with higher level than log max value");

	/* On the fly change parameters (enable file logger) */
	SlogConfig slgCfg;
	slog_config_get(&slgCfg);
	slgCfg.nToFile = 1;
	slog_config_set(&slgCfg);

	// Print message and save log in the file 
	slog(0, SLOG_DEBUG, "Debug message in the file with int argument: %d", int_arg);

	// On the fly change parameters (enable file logger) 
	slog_config_get(&slgCfg);
	slgCfg.nPretty = 1;
	slog_config_set(&slgCfg);

	// Print message and save log in the file without colorization
	slog(0, SLOG_DEBUG, "Debug message with disabled pretty log");

    return 0;
}