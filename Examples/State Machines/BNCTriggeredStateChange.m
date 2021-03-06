% Example state matrix: Switches states when a TTL pulse arrives on BNC trigger channel 1


<<<<<<< HEAD
sma = NewStateMatrix();
=======
sma = NewStateMachine();
>>>>>>> 9dd1b1005e57c9d9ff3bf3532524677d5dfa1801

sma = AddState(sma, 'Name', 'Port1LightOn', ...
    'Timer', 1,...
    'StateChangeConditions', {'BNC1High', 'Port3LightOn'},...
    'OutputActions', {'PWM1', 255});
sma = AddState(sma, 'Name', 'Port3LightOn', ...
    'Timer', 1,...
    'StateChangeConditions', {'Tup', 'exit'},...
    'OutputActions', {'PWM3', 255});