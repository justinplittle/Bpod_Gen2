void tick_func(void);
extern void triggerSchedWave(unsigned wave_id);
extern double readAI(unsigned chan);
extern unsigned state();
extern void logValue(const char *varname, double val);
extern int writeDIO(unsigned chan, unsigned bitval);
TRISTATE thresh_func(int chan, double v);
void init_func(void);
void start_trial_func(void);

/********************************************************************************
 * TEXT BELOW WILL BE FIND-REPLACED FROM MATLAB PRIOR TO SetStateProgram()
 ********************************************************************************/
const double touch_thresh_high = XXX;
const double touch_thresh_low = XXX;

const double numCycThresh = XXX;
const double numCycVal = XXX;

static unsigned vVect1[XXX] = {XXXX};
static unsigned vVect2[XXX] = {XXXX};

static unsigned states_to_log_touch[XXX] = {XXX}; /*list of states in which we want to detect touches for SM*/

/********************************************************************************
 * END: FIND-REPLACE TEXT
 ********************************************************************************/
int cycCounter = 1;

double v_state;
double v_state_last = 0;

/*vars for sensor 1*/
double v_state1;
double v_state1_last = 0;
double std1 = 0;
double mean1;
double sumV1;
double sumV1_sq;
double baseMean1;

/* vars for sensor 2*/
double v_state2;
double v_state2_last = 0;
double std2 = 0;
double mean2;
double sumV2;
double sumV2_sq;
double baseMean2;


void tick_func(void)
{
  	struct wave_id_list { /* scheduled wave IDs  REPLACE WITH ENUM*/
 		unsigned touch_onsets;
 	};
 	struct wave_id_list wave_ids = {.touch_onsets = 0};

 	struct varlog_val_list {
 		double touch_trig_on;
		double touch_trig_pro_on;
		double touch_trig_ret_on; 
 		double touch_trig_med_on;
 		double touch_trig_lat_on;
		double touch_trig_off;
 		double touch_trig_pro_off;
		double touch_trig_ret_off;
 		double touch_trig_med_off;
 		double touch_trig_lat_off;
	};
 	struct varlog_val_list varlog_vals = {.touch_trig_on = 1.0, .touch_trig_off = 2.0, .touch_trig_pro_on = 3.0, .touch_trig_pro_off = 4.0, .touch_trig_ret_on = 5.0, .touch_trig_ret_off = 6.0, .touch_trig_med_on = 7.0, .touch_trig_med_off = 8.0, .touch_trig_lat_on = 9.0, .touch_trig_lat_off = 10.0};

 	const unsigned touch_detector_ai_chan1 = 7; /* Analog input channel for axial sensor. */
  	const unsigned touch_detector_ai_chan2 = 9; /* Analog input channel for axial sensor. */

  	double v1;
 	double v2;
	
 	unsigned curr_state = state();
 	unsigned i;

 	int n_touch_state;
 	int in_touch_state;

	/*reset some tracking buffers*/
  	sumV1 = 0;
 	sumV2 = 0;
 	sumV1_sq = 0;
 	sumV2_sq = 0;

	/*read the touch sensor voltage and store in cycle buffer*/
  	v1 = readAI(touch_detector_ai_chan1);
	v2 = readAI(touch_detector_ai_chan2);

	/*keep filling until the cycle buffer for std and thresh val has been filled. Once full, start looping it*/
  	if (cycCounter <= numCycThresh + numCycVal){
		vVect1[cycCounter] = v1;
 		vVect2[cycCounter] = v2;
 	}
 	
 	if (cycCounter > numCycThresh + numCycVal) {
 		for (i = 0; i <= numCycThresh + numCycVal - 2; i++) { 
			vVect1[i] = vVect1[i+1];
			vVect2[i] = vVect2[i+1];
  		}
		
 		vVect1[sizeof(vVect1) - 1] = v1; 
 		vVect2[sizeof(vVect1) - 1] = v2; 
	}
	
	/* if we arent in a state where we care about touches controlling SM transitions, dont bother with touch detection code*/
	n_touch_state = sizeof(states_to_log_touch);

	for (i = 0; i <= n_touch_state - 1; i++) {
		if (curr_state == states_to_log_touch[i]) {
			in_touch_state = 1;
		}
	}
	
	if (curr_state  >= 45) {
	/*get the standard deviation of the v measuremnts for the last numCycThresh measures*/
		for (i = 0; i <= numCycThresh - 1; i++) {
			sumV1 = sumV1 + vVect1[i];			
			sumV2 = sumV2 + vVect2[i];
		}
		
		baseMean1 = (sumV1 / numCycThresh);
		baseMean2 = (sumV2 / numCycThresh);
 		
		for (i = 0; i <= numCycThresh - 1; i++) {
			std1 = std1 + (vVect1[i] - baseMean1) * (vVect1[i] - baseMean1);			
			std2 = std2 + (vVect2[i] - baseMean2) * (vVect1[i] - baseMean1);
			 
		}           
		
		std1 = (std1 / numCycThresh);
		std2 = (std2 / numCycThresh);

		/*get the mean of the v measuremnts for the last numCycVal measures*/
		sumV1 = 0;
		sumV2 = 0;
		sumV1_sq = 0;
		sumV2_sq = 0;
		
		for (i = numCycThresh; i <= numCycThresh + numCycVal - 1; i++) {
			sumV1 = sumV1 + vVect1[i];
			sumV2 = sumV2 + vVect2[i];
		}
		
		mean1=(sumV1 / numCycVal) - baseMean1;
	    mean2=(sumV2 / numCycVal) - baseMean2;
 
		/*thresholding for sensor 1*/
		if (mean1 >= std1 * touch_thresh_high) {
			v_state1 = 1;  
		} else if (mean1 <= std1 * touch_thresh_low) {
			v_state1 = 2;  
			mean1 = mean1 * -1; 
		}

		/*thresholding for sensor 2*/
		if (mean2 >= std2 * touch_thresh_high) {
			v_state2 = 1;  
		} else if (mean2 <= std2 * touch_thresh_low) {
			v_state2 = 2;   
			mean2 = mean2 * -1; 
		}

		/*thresholding for combined sensor voltages*/
		if (mean1 + mean2 >= (std1 + std2) * touch_thresh_high) {
			v_state = 1;   

		} else {
			v_state = 0;
		}
		
		/*touch classification and output triggering*/
		if (v_state1 == 1 && v_state1_last != 1) { /*begin touch onest */
			logValue("touch_trig_pro_on", varlog_vals.touch_trig_pro_on); /* Log that touch onset */
		} else if (v_state1 != 1 && v_state1_last == 1) { /*begin touch offest */
			logValue("touch_trig_pro_off", varlog_vals.touch_trig_pro_off); /* Log that touch onset */
		} else if (v_state1 == 2 && v_state1_last != 2) { /*begin touch onest */
			logValue("touch_trig_ret_on", varlog_vals.touch_trig_ret_on); /* Log that touch onset */
		} else if (v_state1 != 2 && v_state1_last == 2) {  /*begin touch offest */
			logValue("touch_trig_ret_off", varlog_vals.touch_trig_ret_off); /* Log that touch onset */
		} else if (v_state2 == 1 && v_state2_last != 1) { /*begin touch onest */
			logValue("touch_trig_med_on", varlog_vals.touch_trig_med_on); /* Log that touch onset */
		} else if (v_state2 != 1 && v_state2_last == 1) { /*begin touch offest */
			logValue("touch_trig_med_off", varlog_vals.touch_trig_med_off); /* Log that touch onset */
		} else if (v_state2 == 2 && v_state2_last != 2) { /*begin touch onest */
			logValue("touch_trig_lat_on", varlog_vals.touch_trig_lat_on); /* Log that touch onset */
		} else if (v_state2 != 2 && v_state2_last == 2) { /*begin touch offest */
			logValue("touch_trig_lat_off", varlog_vals.touch_trig_lat_off); /* Log that touch onset */
		} else if (v_state == 1 && v_state_last != 1) {  /*begin touch onest */
			logValue("touch_trig_on", varlog_vals.touch_trig_on); /* Log that touch onset */
			triggerSchedWave(wave_ids.touch_onsets);  /*trigger touch sched wave*/
		} else if (v_state != 1 && v_state_last == 1) { /*begin touch offest */
			logValue("touch_trig_off", varlog_vals.touch_trig_off); /* Log that touch onset */
			untriggerSchedWave(wave_ids.touch_onsets);  /*untrigger touch sched wave*/
		}
		
		if (v_state == 1 && v_state_last != 1) {  /*begin touch onest */
			logValue("touch_trig_on", varlog_vals.touch_trig_on); /* Log that touch onset */
			triggerSchedWave(wave_ids.touch_onsets);  /*trigger touch sched wave*/
	    }
		
		v_state1_last = v_state1;
		v_state2_last = v_state2;
		v_state_last = v_state;		
	}	
	else { /*ends the section for touch detection and schedling trigg waves*/

		v_state1_last = 0;
		v_state2_last = 0;
		v_state_last = 0;
	}
	/*update the cyle counter*/
 	cycCounter = cycCounter + 1;

}

void start_trial_func(void)
{
	logValue("entered_state_40", 1.0); /* Useful time stamp. */
	
    unsigned i = 1;
	/* Fill a vector of zeros to keep track of touch sensor v of the last numCycThresh+numCycVal cycles */
	for (i = 0; i < numCycThresh + numCycVal - 1; i++) {
	 	vVect1[i] = 0;
	 	vVect2[i] = 0;
		
	}

}

void init_func(void)
{
	unsigned i = 1;
	/* Fill a vector of zeros to keep track of touch sensor v of the last numCycThresh+numCycVal cycles */
	for (i = 0; i < numCycThresh + numCycVal - 1; i++) {
	 	vVect1[i] = 0;
	 	vVect2[i] = 0;
		
	}
		
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
	if (chan == 0 || chan == 1 || chan == 7 || chan == 9) { /* Lickport input channels = hardware channels 0 and 1*/
		if (v >= 4.0) return POSITIVE;  /* if above 4.0 V, above threshold */
		if (v <= 3.0) return NEGATIVE;  /* if below 3.0, below threshold */
		return NEUTRAL; /* otherwise unsure, so no change */
	} else {
		return NEUTRAL; /* Do not allow "beam-break" events on non-lickport channel */
	}
}
