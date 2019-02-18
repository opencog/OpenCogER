/* 
 * File: SSTT.cpp
 * Author: Dagim Sisay <dagiopia@gmail.com>
 * License: AGPL
 * Date: May, 2018
*/

#include <errno.h>
#include "sense/audio/STT.hpp"

// Data
void (*send_text)(const char *s); //user defined func
void (*get_audio) (void *buffer, uint32_t size); //func_a
//std::thread run;
bool is_running;
ps_decoder_t *ps;
cmd_ln_t *cfg;
ad_rec_t *ad;
uint8 utt_started, in_speech;
char const *hyp;

bool server;

//AudioCap
AudioCap *ac;

//socket
int fd, cl, ret, sock_fd;
struct sockaddr_un addr;



void init_ps()
{
	is_running = false;
	cfg = cmd_ln_init(NULL, ps_args(), TRUE,
	                  "-hmm", "/usr/local/share/pocketsphinx/model/en-us/en-us",
	                  "-lm", "/usr/local/share/pocketsphinx/model/en-us/en-us.lm.bin",
	                  "-dict", "/usr/local/share/pocketsphinx/model/en-us/cmudict-en-us.dict",
	                  "-remove_noise", "yes",
	                  "-logfn", "/dev/null",
	                  NULL);
	if( cfg == NULL) { fprintf(stderr, "Error Creating Config Object\n"); exit(1); }
	ps = ps_init(cfg);
	if (ps == NULL) { fprintf(stderr, "Error Initializing Recognizer\n"); exit(1); }
	
	/* start ps utterance */
	if (ps_start_utt(ps) < 0) {
		fprintf(stderr, "Error Starting Utterance\n");
		exit(1);
	}
	 
	utt_started = FALSE;
	printf("Ready......\n"); //XXX Remove when done
}

void init_ad()
{
	ad = ad_open();
	if(ad == NULL) { fprintf(stderr, "Error Opening Device\n"); exit(1); }
	if(ad_start_rec(ad) < 0) {
		fprintf(stderr, "Error Starting Recording\n");
		exit(1);
	}
}


void init_ac()
{
	ac = new AudioCap();
	ac->set_callback(ac_callback);
	ac->start();
}


void init_socket(std::string address, bool client)
{
	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(fd == -1) { fprintf(stderr, "Error Creating Socket\n"); exit(1); }

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, address.c_str(), sizeof(addr.sun_path)-1);

	if(client){
		if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
			fprintf(stderr, "Error Connecting to Socket: %s\n", addr.sun_path); perror("");
			exit(1);
		}
		sock_fd = fd;
	}
	else {
		if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
			fprintf(stderr, "Error Binding Address to Socket:%s\n", 
			                 address.c_str()); perror("");
			exit(1);
		}
		if(listen(fd, 2) == -1) {
			fprintf(stderr, "Error Listening on Socket:%s\n", address.c_str());
			perror("");
			exit(1);
		}
		if((ret = accept(fd, NULL, NULL)) == -1) {
			fprintf(stderr, "Error Accepting Connection: %d\n", ret); perror("");
			exit(1);
		}
		sock_fd = ret;
	}
	server = !client;
	send_text = socket_send;
}


void tcstt_init(bool ps_ad, void (*f)(const char *s))
{
	init_ps();
	if(ps_ad)
		init_ad();
	else
		init_ac();

	send_text = f; //set the callback function
	
	is_running = true;

	if(ps_ad) {
		std::thread run(tcstt_run);
		run.join();
	}
}


void tcstt_init(bool ps_ad, std::string address, bool client)
{
	init_ps();
	//init socket
	init_socket(address, client);
	
	if(ps_ad)
		init_ad();
	else
		init_ac();

	is_running = true;

	if(ps_ad) {
		std::thread run(tcstt_run);
		run.join();
	}
}



void tcstt_close()
{
	 is_running = false;
//    delete run;
}


bool tcstt_is_on()
{
	 return is_running;
}


void ac_callback(void *buffer, uint32_t samples)
{
	ps_process_raw(ps, (const int16_t*)buffer, samples, FALSE, FALSE);
	in_speech = ps_get_in_speech(ps);
	if (in_speech && !utt_started) { 
		utt_started = TRUE;
		printf("Listening...\n"); //XXX Remove when done
	}
	if (!in_speech && utt_started) {
		ps_end_utt(ps);
		hyp = ps_get_hyp(ps, NULL);
		if (hyp != NULL)
			send_text(hyp); //printf("Text: %s\n", hyp);
			if (ps_start_utt(ps) < 0)
				fprintf(stderr, "Error Starting Utterance\n");
		utt_started = FALSE;
		printf("Ready.....\n"); //XXX Remove when done
	}
}


void socket_send(const char *s)
{
	ret = write(sock_fd, s, strlen(s));
	if(ret < 0) {
		fprintf(stderr, "Error write text to socket: "); perror("");
	}
}


void tcstt_run()
{
	 int32 k;
	 int16 adbuf[2048];
	 while (is_running) {
		  if ((k = ad_read(ad, adbuf, 2048)) < 0)
				fprintf(stderr, "Errpr Reading Audio Data\n");
		  ps_process_raw(ps, adbuf, k, FALSE, FALSE);
		  in_speech = ps_get_in_speech(ps);
		  if (in_speech && !utt_started) { 
				utt_started = TRUE;
				printf("Listening...\n"); //XXX Remove when done
		  }
		  if (!in_speech && utt_started) {
				ps_end_utt(ps);
				hyp = ps_get_hyp(ps, NULL);
				if (hyp != NULL)
					 send_text(hyp);  //send text
				if (ps_start_utt(ps) < 0)
					 fprintf(stderr, "Error Starting Utterance\n");
				utt_started = FALSE;
				printf("Ready....\n"); //XXX Remove when done;
		 }
	}
}
