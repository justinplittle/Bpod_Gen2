
void tick_func(void);
extern void triggerSchedWave(unsigned wave_id);
extern void logValue(const char *varname, double val);
extern void triggerSound(unsigned card, unsigned snd);
extern void untriggerSound(unsigned card, unsigned snd);
extern double ceil(double);
extern double floor(double);

/*extern struct EmbCTransition transition()
 */
extern double sqrt(double);
extern unsigned state();
extern int forceJumpToState(unsigned state, int event_id_for_history);
extern double readAI(unsigned chan);
extern int writeDIO(unsigned chan, unsigned bitval);
extern int writeAO(unsigned chan, double voltage);
extern int bypassDOut(unsigned bitmask);

TRISTATE thresh_func(int chan, double v);
void init_func(void);
void enter_state_40(void);
void enter_state_61(void);
void enter_state_62(void);
void enter_state_63(void);
void enter_state_64(void);
void enter_state_65(void);
void enter_state_66(void);
void enter_state_67(void);
void enter_state_68(void);
void enter_state_69(void);
void enter_state_70(void);
void enter_state_71(void);
void enter_state_72(void);
void enter_state_73(void);
void exit_state_67(void);

/********************************************************************************
 * TEXT BELOW WILL BE FIND-REPLACED FROM MATLAB PRIOR TO SetStateProgram()
 ********************************************************************************/
/*THRESHOLD SECTION*/
double touch_thresh = XXX;
/*TIMING SECTION*/
double pre_pole_delay = XXX;
double resonance_delay = XXX;
double answer_delay = XXX;
double delay_period = XXX;
double sample_period = XXX;
double answer_period = XXX;
double drink_period = XXX;
int mean_window_length = XXX;
int median_window_length = XXX;
int baseline_length = XXX;
double vVect1[XXX];
double vVect2[XXX];
double tmpMeanVect[XXX];
double tmpMedianVect[XXX];
double vbaseVect1[XXX];
double vbaseVect2[XXX];
double log_analog_freq = XXX;
double valve_time = XXX;
/*SOUNDS SECTION*/
int go_cue = XXX;
int fail_cue = XXX;
int pole_cue = XXX;
int rew_cue = XXX;
/********************************************************************************
 * END: FIND-REPLACE TEXT
 ********************************************************************************/
double mean1 = 0;
double median1 = 0;
double basemedian1 = 0;
double last_mean1 = 0;
double last_sum1 = 0;
double last_median1 = 0;
double mean2 = 0;
double last_mean2 = 0;
double last_median2 = 0;
double basemean1 = 0;
double last_basemean1 = 0;
double baseline = 0;
double real_baseline = 0;
double std1;

int first_touch_time = 0;
int num_touches = 0;
int pre_pole_time = 0;
int pole_up_time = 0;
int rew_cue_time = 0;
int go_cue_time = 0;
int pole_cue_time = 0;
int delay_period_time = 0;
int sample_period_time = 0;
int resonance_time = 0;
int answer_time = 0;
int lickport_time = 0;
int rew_time = 0;
int start_drink_time = 0;
int valve_flag = 0;
int blerg = 0;
int blerger = 0;
int size;

int rew_cue_flag = 0;
int touch_state_flag = 0; 
int delay_touch_state_flag = 0;

int log_analog_window_counter = 1;
int cycle_counter = 1;
int mean_window_counter = 1;
int debug_counter = 1;
int baseline_counter = 1;

int t = 1;
int v_state;
int v_state_last = 0;
int v_state1;
int v_state1_last = 0;
int v_state2;
int v_state2_last = 0;
int size;

unsigned curr_state;
unsigned last_state = 40;

const unsigned lick_left = 0; /* Analog input channel for left lick port. */
const unsigned touch_detector_ai_chan1 = 7; /* Analog input channel for axial sensor. */
const unsigned touch_detector_ai_chan2 = 9; /* Analog input channel for radial sensor. */

/*TICK FUNCTION TO RUN EACH CYCLE*/
void tick_func(void) {
	
	double lick;
	double v1;
	double v2;
	unsigned curr_state = state();
	unsigned i;
	
	/*update state*/
	curr_state = state();
	
	/*now update the voltage readings*/
	lick = readAI(lick_left);
	v1 = readAI(touch_detector_ai_chan1);
	/*v1=v1-0.002; /*offset correction, see Ephus*/
	
	/*local baseline subtraction estimate..do NOT update during pole moves!*/
	/*fill the circ. buffers for the baseline measurements*/
	if (curr_state < 62 ) {
	
		if (baseline_counter > baseline_length - 1) {
			baseline_counter  = 0;
			vbaseVect1[baseline_length-1] = v1;
		} else {
			vbaseVect1[baseline_length-1] = v1;
		}
		double bsum1 = 0;
		for (i = 0; i < baseline_length - 1; i++) {
			bsum1 = bsum1 + vbaseVect1[i];
		}
	
		real_baseline = bsum1/baseline_length;
	} 
	 
	/*fill the circ. buffers for the signal measurements*/
	if (mean_window_counter > mean_window_length) {
		mean_window_counter  = 0;
		vVect1[mean_window_length-1] = v1;
	} else {
		vVect1[mean_window_counter-1] = v1;
	}

    /*MEDIAN FILTERING*/
	/*home-made sort since we have no libraries*/
	/******************************************/
	/*SORT SIGNAL WINDOW DATA*/

	int j, p, z, imin; 
	double tmp;	
	for (z = 0 ; z < mean_window_length - 1; z++){ /*step the small median window, zero-padded orig vect at end*/
		for (j = 0 ; j < median_window_length - 1 ; j++){  /*loop through the small median window*/
			tmpMedianVect[j]=vVect1[j+z];
		}
		for (j = 0 ; j < median_window_length - 1 ; j++){  /*loop through the small median window*/
			imin = j;
			for (p = j + 1; p < median_window_length ; p++){
				if (tmpMedianVect[p] < tmpMedianVect[imin]){
					imin = p;
				}
			}
			tmp = tmpMedianVect[j];
			tmpMedianVect[j] = tmpMedianVect[imin];
			tmpMedianVect[imin] = tmp;
		}
	
		/*END SORT*/
		/******************************************/	

		/*compute median of voltages in signal windows*/
		if (median_window_length % 2 == 0) {
			median1 = (tmpMedianVect[median_window_length / 2 - 1] + tmpMedianVect[median_window_length / 2]) / 2;
		}
		else  {
		median1 = tmpMedianVect[median_window_length / 2];
		}
		/*median filter*/
		for (i = 0; i < median_window_length; i++) {
			tmpMedianVect[i] = median1;
		}
		/*assign this value back into the proper section of the original vector*/
		for (j = 0 ; j < median_window_length - 1 ; j++){  /*loop through the small median window*/
			tmpMeanVect[j+z]=median1;
		}
	}
	
	/*compute mean over the median-filtered average window*/
    double sum1 = 0;
	for (i = 0; i < mean_window_length - 1; i++) {
		sum1 = sum1 + tmpMeanVect[i];
	}
	mean1 = sum1/mean_window_length;
	
	/*compute sum f squares of voltages from LAST cycle*/
    /*the baseline vect is 2x the length of the signal vect, so use tyhe first half of that*/

	double sum1sq = 0;
	double vBase = 0;
	for (i = 0; i < mean_window_length; i++) {
		sum1sq = sum1sq + (vbaseVect1[i]*vbaseVect1[i]);
		vBase=vBase+vbaseVect1[i];
	}
	vBase=vBase/mean_window_length;
	std1 = sqrt((sum1sq - vBase)*(sum1sq - vBase));

	double absdiff1 = sqrt((sum1 - last_sum1)*(sum1 - last_sum1));
	
	/*thresholding for sensor 1 sensors*/
	if (absdiff1 > 0.03 && v_state_last == 0)  { 
		v_state = 1;
		if (curr_state == 64 || curr_state == 65) { /*only log touches during delay and sample periods*/
			if (curr_state == 65) {
				touch_state_flag = 1;
				logValue("touch", 1.0);
				if (blerg == 0) {
					first_touch_time = cycle_counter;
					/*first touch is special. note its time*/
					blerg = 1;
				}
			} else if (curr_state == 64) {
				delay_touch_state_flag = 1;
				/*logValue("delay_touch", 1.0);*/
			}
		}
	}
	
	else {
		v_state = 0;
	}
	

	/*STATE MATRIX CODE */

	/*state 40 is just an entry func*/
	if (curr_state == 40) {
		if (blerger == 0) {
			logValue("entered_state_40", 1.0); /* Useful time stamp. */
			bypassDOut(1024); /*send the trigger for ephus/si, cameras, etc*/
			triggerSound(0, pole_cue); /*trigger the pole cue*/
			pole_cue_time = cycle_counter;
			blerger = 1;
			forceJumpToState(41, 2); 
		}
			/*statement to end the trigger pulse after 5ms*/
		if (cycle_counter - pole_cue_time >= 60) {
			bypassDOut(0);
		}
	}
	
	
    /*state 60 is last bitcode state*/
	if (curr_state == 60) { 
		forceJumpToState(61, 3);
		curr_state = 61;
	}
	

    /*state 61 is a pre-pole delay time*/
	if (curr_state == 61) { 
		if  (cycle_counter > pre_pole_delay) {
			forceJumpToState(62, 3);
			curr_state = 62;
		}
	}

	/*state 62 is just an entry function*/
	
	/*state 63 is a hard delay for sensor resonance timeout*/
	if (curr_state == 63) { 
		if ((cycle_counter - resonance_time) > resonance_delay) {/*at ^k sampling, should be 500ms delay*/
			forceJumpToState(64, 5);
			curr_state = 64;
		}
	}

	/*state 64 is the variable delay period*/
	if (curr_state == 64) { 
		if ((cycle_counter - delay_period_time) > delay_period) {  /*if delay period is over...*/
			forceJumpToState(65, 6);  /*sound go cue and move to sample period*/
			/*exit func takes care of go cue here*/
			curr_state = 65;
		} else {
			if (delay_touch_state_flag == 1) { 
				forceJumpToState(72, 12);  /*touched during the delay, timeout*/
				/*entry func takes care of fail cue here*/
			} 
		}
	}
	

	/*state 65 is the sampling period*/
	if (curr_state == 65) {
		if ((cycle_counter - sample_period_time) < sample_period) { /*if we have not reached the end of the sample period*/
			if (touch_state_flag == 1) { /*if there has been a touch*/
				forceJumpToState(66, 7); /*go to answer delay*/
			} 
		} else if ((cycle_counter - sample_period_time) >= sample_period) { /*end of sampling period reached*/
			bypassDOut(0); /*remove the pole*/
		}
	}
	
	if (curr_state > 64 && curr_state <= 69) { /*only if we are in or beyond the sampling period*/
		if ((cycle_counter - sample_period_time) > sample_period) { /*end of sampling period reached*/
			/*nothing here for the moment*/
		
			/*if a reward was not delivered, go to miss state*/
			if (rew_cue_flag == 0){
				forceJumpToState(71, 11);
			}
		}
	}

	
	/*state 66 is a variable delay before the reward cue is played and answer period entered*/
	if (curr_state == 66) {
		if (lick > 4.0) { /*early lick - end trial*/
			lickport_time = cycle_counter;
			forceJumpToState(72, 12);
		} else {
			
			if ((cycle_counter - first_touch_time) >= answer_delay && rew_cue_flag == 0) {
				forceJumpToState(67, 8); /*go to answer period*/
				/*curr_state = 67;*/
				rew_cue_time = cycle_counter;
				rew_cue_flag = 1;
			}
		}
	}
	

	/*state 67 waits for a fixed amount of time for a lick.*/
	if (curr_state == 67) {
		if ((cycle_counter - rew_cue_time) < answer_period) {  /*still in the answer period*/
			if (lick > 3.0) {
				forceJumpToState(68, 9); /*jump to valve period*/
			}
			
		} else { /*go to miss state*/
			forceJumpToState(71, 11); 
		}
	}
	
	
	/*state 68 is a fixed vale time trigger*/
	if (curr_state == 68) {
		if ((cycle_counter - valve_flag) > valve_time) {  /*still in the valve open period*/
			bypassDOut(0);
			forceJumpToState(69, 10);
		}
	}
	
	/*state 69 is a fixed period where the animal can drink*/	
	if (curr_state == 69) {
		if ((cycle_counter - start_drink_time) > drink_period) {  /*still in the answer period*/
			bypassDOut(0); /*untrigger the valve (if it isnt already)*/
			forceJumpToState(70, 11);	
		}
	}
	
	
	
	/*if sample period has not ended, and we are coming from a hit/miss/timout state, wait*/
	if (curr_state == 70 || curr_state == 71 || curr_state == 72) {
		if ((cycle_counter - answer_time) > answer_period) {
				bypassDOut(0);
				forceJumpToState(74, 12);
		}
	}


	cycle_counter = cycle_counter + 1;
	mean_window_counter = mean_window_counter + 1;	
	log_analog_window_counter = log_analog_window_counter + 1;
	
	/*DEBUGGING SECTION*/
	
	if (debug_counter == 24) {
		/*logValue("absdiff", absdiff1);
		
		logValue("v_state", v_state);	
		*/	 		

		debug_counter = 0;
	}
	
	debug_counter = debug_counter + 1;		
	v_state1_last = v_state1;
	v_state2_last = v_state2;
	v_state_last = v_state;
	last_mean1 = mean1;
	last_mean2 = mean2;
	last_sum1 = sum1;

}

/*STATE EXIT FUNCTIONS*/
/*exiting state 40 jumps to first bitcode state*/
/*void exit_state_40(void) {
	forceJumpToState(101, 16); 
}*/

/*exiting state 67 turns off dout bypas so matlab can trigger lick port*/
/*kind of kludgy*/
void exit_state_67(void) {
	bypassDOut(0); 
}



/*STATE ENTRY FUNCTIONS*/
/*state 40 sends triggers, plays pole cue, and ends*/
void enter_state_40(void) {

	
}

/*state 61 engages in a variable pre-pole delay, then ends*/
void enter_state_61(void) {
	pre_pole_time = cycle_counter;
	logValue("entered_state_61", 1.0); /* Useful time stamp. */
}	

/*state 62 triggers the pole up, flags the time, updates logs/states*/
void enter_state_62(void) {
	pole_up_time = cycle_counter;
	logValue("entered_state_62", 1.0); 
	bypassDOut(16); /*trigger the pole*/
	forceJumpToState(63, 4);
	curr_state = 63;
	resonance_time = cycle_counter;
}

/*state 63 entry triggers a log write, updates current state*/
void enter_state_63(void) {
	logValue("entered_state_63", 1.0); 
}	

/*state 64 entry triggers a log write, updates current state*/
void enter_state_64(void) {
	logValue("entered_state_64", 1.0); 
	delay_period_time = cycle_counter;
}	

/*state 65 entry triggers the go cue and a log write, updates current state*/
void enter_state_65(void) {
	triggerSound(0, go_cue); /*trigger the go cue*/
	logValue("entered_state_65", 1.0); 
	sample_period_time = cycle_counter;
	
}

/*state 66 is a delay period entered after the first touch*/
void enter_state_66(void) {
	logValue("entered_state_66", 1.0);
	
}

/*state 67 is the answer period*/
void enter_state_67(void) {
	logValue("entered_state_67", 1.0);
	triggerSound(0, rew_cue); /*play reward wave*/
	rew_time = cycle_counter;
}


/*state 68 is the valve opening*/
void enter_state_68(void) {
	logValue("entered_state_68", 1.0);
	valve_flag = cycle_counter;
	bypassDOut(256); /* turn the valve on*/
}

/*state 69 is the drink period*/
void enter_state_69(void) {
	logValue("entered_state_69", 1.0);
	start_drink_time = cycle_counter;
}

/*state 70 is hit*/
void enter_state_70(void) {
	logValue("entered_state_70", 1.0);
 }

/*state 71 is miss*/
void enter_state_71(void) {
	logValue("entered_state_71", 1.0);
	forceJumpToState(74, 12);
}

/*state 52 is the timeout state*/
void enter_state_72(void) {
	logValue("entered_state_72", 1.0);
	triggerSound(0, fail_cue); /*play noise wave*/
	forceJumpToState(74, 13); /*go to final state*/
	logValue("entered_state_74", 1.0);
	bypassDOut(0); /*remove the pole*/
}

/*state 53 is a wait period*/
void enter_state_73(void) {
	logValue("entered_state_73", 1.0);
}

/*INIT FUNCTION*/
void init_func(void) {
}



/* Want to configure second analog input channel (beyond lickport channel)
* with SetInputEvents.m in order to (1)
* read in whisker position with readAI(); and (2) to record times of stimulation
* using scheduled waves event triggering. These events get recorded and made
* available to MATLAB as input events on this second channel.  We *don't* however
* want actual input events to get triggered on this channel.  Thus, we re-define
* the built-in threshold detection function in order to detect events *only* on
* the lickport channel.
*/
TRISTATE thresh_func(int chan, double v) 
{
    if (chan == 0 || chan == 1) { /* Lickport input channels = hardware channels 0 and 1*/
        /*if (v >= 4.0) return POSITIVE;  /* if above 4.0 V, above threshold */
        /*if (v <= 3.0) return NEGATIVE;  /* if below 3.0, below threshold */
        
		return NEUTRAL; /* otherwise unsure, so no change */
    }
    else {
        return NEUTRAL; /* Do not allow "beam-break" events on non-lickport channel */
    }
}
