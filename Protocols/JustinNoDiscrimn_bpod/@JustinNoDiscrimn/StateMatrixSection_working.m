% Create and send state matrix.

% Santiago Jaramillo - 2007.08.24
%
%%% CVS version control block - do not edit manually
%%%  $Revision: 1064 $
%%%  $Date: 2008-01-10 16:50:50 -0500 (Thu, 10 Jan 2008) $
%%%  $Source$


%%% BUGS:
%
% [2007.10.04] If the duration of sound is changed, the State Matrix is not
% updated but the sound duration is.  This is a problem if Duration is changed
% from large value to a small one, since there will still be punishment after
% the sound off-set.  Updating the matrix on update_sounds didn't work.


function sma = StateMatrixSection(obj, action)

%call to SetStateProgram occurs outside of this function when using
%dispatcher. Make the embedC matrix global
global G

%set in dispatcher, reflects Dout mappins from settings file
global unsorted_channels
global channels

%need to convert the channel mappings for each DOut to binary, then to
%decimal

%%%---OUTPUT SPECIFICATION

bitcode=12;          %binary = 1100,         bitcode is chan2
lick_1=9;            %binary = 1001,         lick_1 is chan4
lick_2=17;           %binary = 10001,        lick_2 is chan5
festo_trig=65;       %binary = 100001,       festo_trig is chan6
cue_led1=257;        %binary = 10000001,     cue_led1 is chan8
ephus_trig=1025;     %binary = 1000000001,   ephus_trig is chan10
embc_touch_out=4097; %binary = 100000000001, DIO line triggered by embc touch sched wave is chan12


GetSoloFunctionArgs;

switch action
    case 'update',
        %%%%NEXT TRIAL SETTINGS

        %JPL - for some reason, next_type is passed as a soloparamhandle,
        %but all the other next_ type vars are passed as strings
        if next_stim_type=='s'
            stim_trial=true;
        else
            stim_trial=false;
        end

        %set stimulation parameters
        %we arent really doing stim at the moment, but this is where you
        %want to load these settings

        %%%%AO/AI SPECIFCATIONS (EmbedC)

        % -- Delays Section
        %delays in the state machine are coded as multiples of the duty
        %cycle, which is 6 samples per ms. Here we define any delays we
        %wish to implement on AO or AI lines. For example, if we want to
        %change state 600ms after a touch on an AI line, then this value
        %will be 3600

        %eventually need to build this into the GUI, but its here for now

        %what is the sampling period of this state machine?
        taskPeriodsPerMs=6;
        %%%specify delays in ms here
        %%%note: we can make this a variable that %changes state-to-state
        postTouch_wait_period_ms=500;  %wait time after touch
        %postDelayInterupt_wait_period_ms=500; %wait time after touch-out during delay and trial reinit
        %%%convert
        postTouch_wait_period=floor(postTouch_wait_period_ms*taskPeriodsPerMs);
        %postDelayInterupt_wait_period=floor(postDelayInterupt_wait_period_ms*taskPeriodsPerMs);

        % -- States Section
        %specify states during which AO or AI actions will take place. Last
        %entry must be the state the ends a particular (e.g. sampling,
        %delay) period;
        sample_period_states=[41 43 57]; %41=pole ascends, 43=valve triggers, 57=pole descends
        delay_period_states=[55 56 57]; %55=lick during delay, 54=touch during delay, 57=delay period

        % JPL - DHO used this for activating stimulation lasers. Maybe we
        % want to do something like this later, but for now, just using it
        % for state transitions.

        if ~isempty(strfind(next_stim_type,'sample_period'))
            stim_state_for_EmbC=[1 sample_period_states];
        elseif ~isempty(strfind(next_stim_type,'delay_period'))
            stim_state_for_EmbC=[1 delay_period_states];
        elseif ~isempty(strfind(next_stim_type,'on_touch'))
            stim_state_for_EmbC=[0 sample_period_states];
        elseif ~isempty(strfind(next_stim_type,'n'))
            %dummy in case the other stuff is happening
            stim_state_for_EmbC=[1 sample_period_states];
        else
            error('Unrecognized stim type selection');
        end

        %'JPL - StateMatrixSection - need to get the protocol name into here to finish the embedC path, but cant figure it out right now...just doing it manually');
        manual='@pole_detect_jpl_0obj';
        embCPath=[bSettings('get','GENERAL','Protocols_Directory') '\' manual '\' 'embc\'] ;

        %load the string of variables to pass to the FSM
        G=wrap_c_file_in_string([pwd embCPath 'globals.c']);
        %fill in the variables in this string with variables we pass to
        %'preprocessEmbC'

        %JPL - just set touch offset manually here...but eventualyl want a
        %radio button in GUI where we can either set it or measure it
        %TouchOffset=0;

        %G=preprocessEmbC(G,stim_trial,postTouch_wait_period,TouchThreshLow,...
        %    TouchThreshHigh,TouchOffset,stim_state_for_EmbC);
        
         %JPL - testing with DHOs settings to see if we get thids through
         %note - THIS WORKS!!
         %the state machine
         stim_delay_in_task_periods=0;
         stim_dur_in_task_periods=0;
         t_thresh_low=-0.05;
         whisker_pos_thresh_high=0.05;
         stim_state=[0 41 43 57];
         stim_AOM_power=0;
         
         touch_thresh_low=-4.0;
         touch_thresh_high=4.0;
         
%          G=preprocessEmbC_test(G,stim_trial,stim_delay_in_task_periods,...
%             stim_dur_in_task_periods,whisker_pos_thresh_low,...
%             whisker_pos_thresh_high,stim_state,stim_AOM_power);

         G=preprocessEmbC_test(G,touch_thresh_low,touch_thresh_high);

        %%%%REGULAR STATE MACHINE STUFF

        IndNoise = SoundManagerSection(obj, 'get_sound_id', 'noise');
        IndSoundReward = SoundManagerSection(obj, 'get_sound_id', 'reward');
        IndSoundGo = SoundManagerSection(obj, 'get_sound_id', 'go');
        IndSoundInit = SoundManagerSection(obj, 'get_sound_id', 'init');

        %get the id for the pole associated with the current position

        cueId_axial=find(ismember(PolePosnList.axial.name,next_axial_pos));
        cueId_radial=find(ismember(PolePosnList.radial.name,next_radial_pos));

        %if this isnt a mismatch trial, use the proper cue
        if strmatch(next_predict, 'predict') %predicted
            %just load the sound
            IndCueSound = SoundManagerSection(obj, 'get_sound_id', PolePosnList.axial.cue{cueId_axial});

        else %mismatch
            cueId=randsample(numel(PolePosnList.axial.name),1);
            IndCueSound = SoundManagerSection(obj,'get_sound_id', cueId);
        end

        ValveDuration = 0.15;               % seconds

        %this exists as a drop-down menu item in the GUI, but delays here
        rand_pole_delay_pool = [0.5  1  1.5  2];

        if ~strcmp(value(pole_delay), 'random')
            %do nothing
        else
            RandDelay_ind = ceil(length(rand_pole_delay_pool)*rand);
            pole_delay = rand_pole_delay_pool(RandDelay_ind); % set Pole delay time as a random number between 0.5 and 1.5
        end;

        %determine reward side output
        if strmatch(value(next_side),'left') %whats the next side?
            lickOut='Lin';  %code for corectglick port set in assembler code
            lickBad='Rin';  %incorrect lick port
        else
            lickOut='Rin';  %code for left lick port set in assembler code
            lickBad='Lin';  %incorrect lick port
        end

        
        sma = StateMachineAssembler('full_trial_structure');
              
        %%%------SCHEDULED WAVES SECTION-----------------
       
        %%first make a scheldued wave we can use for reporting touch onsets
        %%and offsets detected in the embeddedC code

        %this wave gets triggered and untriggered within globals.c / the
        %embedC functionality. It is triggered when there is a touch
        %(threshold crossing on touch sensor channel)and produces an output
        %on DIO line 12. This gets shuttled via the breakout box to the
        %'TouchIn' input line (14)
        
        %right now this appears to work, but triggers a short time after
        %the start of the trial, regardless of input
        %threshold appears to be wrong?
        [sma, thisName, thisID] = add_scheduled_wave(sma,...
            'name','touch_onsets',...
            'sustain',0.01,...
            'dio_line', 12);

        %wave with same 'In' event timing as the delay wave, but then
        %causes an 'Out' event after the sample period time + go cue time
        [sma, thisName, thisID] = add_scheduled_wave(sma,'name', 'PoleUpWave',...
            'sustain',DelayPeriodTime+SamplingPeriodTime+GoCueDur,...
            'dio_line',6);

        %wave that runs for the delay period time, then causes an 'In'
        %event, and plays the go cue, and triggers an end event after the0
        %end of the cue
        [sma, thisName, thisID] = add_scheduled_wave(sma,'name', 'delayWave',...
           'preamble', 0.02,... %for pole raising
           'sustain',DelayPeriodTime);
        
        %wave that triggers the pole cue at the beginning of the trial
        [sma, thisName, thisID] = add_scheduled_wave(sma,'name', 'cueWave',...
            'preamble',0.002,...
            'sustain',PoleCueDur-0.001,...
            'sound_trig', IndCueSound);
        
        %wave that triggers the pole cue at the beginning of the trial
        [sma, thisName, thisID] = add_scheduled_wave(sma,'name', 'goWave',...
            'preamble',0.001,...
            'sustain',GoCueDur-0.001,...
            'sound_trig', IndSoundGo);


        %MAIN STATE MACHINE DEFINITION

        %%-----START------------------------------------------------------
        %send bitcode and start
        sma = add_state(sma, 'name', 'start', ...
            'self_timer', 0.001,...
            'output_actions',{'DOut',bitcode},...
            'input_to_statechange', {'Tup', 'ephus_trig'});
        
       %send ephus trigger signal outs
        sma = add_state(sma, 'name', 'ephus_trig', ...
            'self_timer', 0.001,...
            'output_actions',{'DOut',ephus_trig},...
            'input_to_statechange', {'Tup', 'play_cue'});

        %%play pole cue,
        %sma = add_state(sma, 'name', 'play_cue', ...
        %    'self_timer', PoleCueDur,...
        %    'output_actions', {'SoundOut', IndCueSound},...
        %   'input_to_statechange', ...
        %    {'Tup', 'pre_pole_time'});

        sma = add_state(sma, 'name', 'play_cue', ...
          'output_actions', {'SchedWaveTrig','cueWave'}, ...
          'input_to_statechange', {'cueWave_Out', 'pre_pole_time'});

        %%------POLE UP, DELAY, AND SAMPLING SECTION-----------------------------

        sma = add_state(sma, 'name', 'pre_pole_time', ...
            'self_timer', pole_delay,...
            'input_to_statechange', {'Tup','raise_pole'});

        %trigger a voltage-high to raise the festo pole. Wait for the 'In'
        %event from delay sched. wave, and then trigger the go cue from the
        %scheduled wave In event
        
        sma = add_state(sma, 'name', 'raise_pole', ...
           'self_timer', 0.02,...
           'output_actions', {'SchedWaveTrig','PoleUpWave'}, ...
           'input_to_statechange', {'touch_onsets_In','timeout';...
           'Tup','delay'});
        
        %sma = add_state(sma, 'name', 'delay', ...
        %   'output_actions', {'SchedWaveTrig','delayWave'}, ...
        %   'input_to_statechange', {'touch_onsets_In','timeout';...
        %   'delayWave_In','cue_and_sample'});
        
        sma = add_state(sma, 'name', 'delay', ...
           'output_actions', {'SchedWaveTrig','delayWave'}, ...
           'input_to_statechange', {'touch_onsets_In','timeout';...
           'delayWave_In','cue_and_sample'});

       %triggers when the (veru brief) duration of the touch sched wave
       %turns off
        sma = add_state(sma, 'name', 'cue_and_sample', ...
           'output_actions', {'SchedWaveTrig','goWave'}, ...
           'input_to_statechange', {'touch_onsets_Out','touch';...
           'PoleUpWave_Out','notouch'});

        %play go cue
%         sma = add_state(sma, 'name', 'go_cue', ...
%             'self_timer', GoCueDur,...
%             'output_actions', {'SoundOut', IndSoundGo},...
%             'input_to_statechange', {'Tup', 'wait_for_touch'});


%         sma = add_state(sma, 'name', 'wait_for_touch', ...
%             'self_timer', SamplingPeriodTime,...
%             'input_to_statechange', ...
%             {'PoleUpWave_Out', 'timeout',...
%             'TouchIn', 'wait_for_sample_end'});

       %if the touch onset scheduled wave (triggered in embedC) gives an In
       %event, then there has been a touch registered. Wait for the end of
       %the sampling period. Otherwise, go to no-touch state
        
        sma = add_state(sma, 'name', 'touch', ...
            'input_to_statechange', ...
            {'PoleUpWave_Out', 'answer_delay'});

        sma = add_state(sma, 'name', 'answer_delay', ...
            'self_timer',AnswerDelayDur,...
            'input_to_statechange', ...
            {'Tup', 'answer_period'});
        
         %play reward cue
         sma = add_state(sma, 'name', 'rew_cue', ...
             'output_actions', {'SoundOut', IndSoundReward},...
             'self_timer', RewCueDur,...
             'input_to_statechange', {'Tup', 'answer_period'});

        if strmatch(next_type,'go')

            sma = add_state(sma, 'name', 'answer_period', ...
                'self_timer',AnswerPeriodTime,...
                'input_to_statechange', ...
                {'Tup', 'miss';lickOut,'hit';lickBad,'miss'});
        else
            sma = add_state(sma, 'name', 'answer_period', ...
                'self_timer',AnswerPeriodTime,...
                'input_to_statechange', ...
                {'Tup', 'correct_reject';lickOut,'false_alarm';lickBad,'false_alarm'});
        end


        %--------FINAL STATE AND CLEANUP--------------------------

        %assign what happend during this trial to various end states that
        %will help compute peformance, d', etc

        % NO TIMEOUT END STATES
        %if we are advancing trials on touches and licks

        sma = add_state(sma, 'name', 'hit', ...
            'self_timer', 0.05,...
            'output_actions',{'DOut',lick_1},...
            'input_to_statechange', {'Tup', 'final_state'});
        sma = add_state(sma, 'name', 'miss', ...
            'self_timer', 0.1,...
            'output_actions', {'SoundOut', IndNoise},...
            'input_to_statechange', {'Tup', 'final_state'});
        sma = add_state(sma, 'name', 'false_alarm', ...
            'self_timer', 0.1,...
            'output_actions', {'SoundOut', IndNoise},...
            'input_to_statechange', {'Tup', 'final_state'});
        sma = add_state(sma, 'name', 'correct_reject', ...
            'self_timer', 0.5,...
            'output_actions',{'DOut',lick_1},...
            'input_to_statechange', {'Tup', 'final_state'});
        sma = add_state(sma, 'name', 'timeout', ...
            'self_timer', 0.01,...
            'input_to_statechange', {'Tup', 'final_state'});
        sma = add_state(sma, 'name', 'notouch', ...
            'self_timer', 0.01,...
            'input_to_statechange', {'Tup', 'final_state'});
        
        sma = add_state(sma, 'name', 'final_state', ...
            'self_timer', 0.01, 'input_to_statechange', {'Tup', 'check_next_trial_ready'});



        dispatcher('send_assembler', sma, 'final_state');

end %%% SWITCH action
