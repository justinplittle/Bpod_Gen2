% Example state matrix: Two states

<<<<<<< HEAD
sma = NewStateMatrix();
=======
sma = NewStateMachine();
>>>>>>> 9dd1b1005e57c9d9ff3bf3532524677d5dfa1801

sma = AddState(sma, 'Name', 'Port1Lit', ...
    'Timer', 1,...
    'StateChangeConditions', {'Tup', 'Port3Lit'},...
    'OutputActions', {'PWM1', 255});
sma = AddState(sma, 'Name', 'Port3Lit', ...
    'Timer', 1,...
    'StateChangeConditions', {'Tup', 'exit'},...
    'OutputActions', {'PWM3', 255});