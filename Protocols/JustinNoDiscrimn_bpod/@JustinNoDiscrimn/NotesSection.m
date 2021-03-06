% [x, y] = NotesSection(obj, action, x, y)
%
% Text box for taking notes during the experiment.
%
%
% PARAMETERS:
% -----------
%
% obj      Default object argument.
%
% action   One of:
%            'init'      To initialise the section and set up the GUI
%                        for it;
%
%            'reinit'    Delete all of this section's GUIs and data,
%                        and reinit, at the same position on the same
%                        figure as the original section GUI was placed.
%           
%            Several other actions are available (see code of this file).
%
% x, y     Relevant to action = 'init'; they indicate the initial
%          position to place the GUI at, in the current figure window
%
% RETURNS:
% --------
%
% [x, y]   When action == 'init', returns x and y, pixel positions on
%          the current figure, updated after placing of this section's GUI.
%
% x        When action = 'get_next_side', x will be either 'l' for
%          left or 'r' for right.
%

function [x, y] = NotesSection(obj, action, x, y)

GetSoloFunctionArgs;


switch action

    case 'init',   % ------------ CASE INIT ----------------
        
        % Save the figure and the position in the figure where we are
        % going to start adding GUI elements:
        fnum=gcf;
        SoloParamHandle(obj, 'my_gui_info', 'value', [x y fnum.Number]); 
        next_row(y);

        %JPL - originally in the savingSection of our old protocols
              EditParam(obj, 'Weight', 'Not recorded', x, y); next_row(y);
      EditParam(obj, 'WeightAfterExp', 'Not recorded', x, y); next_row(y);
      MenuParam(obj, 'HeadFixed', {'Fixed w/o anesthesia','Fixed after anesthesia','Not fixed'},'Fixed after anesthesia', x, y); next_row(y);
      MenuParam(obj, 'Lighting', {'IR Security Cam','Diffuse 940nm for Security Cam','940nm High-speed Imaging','White light','Totally Dark'},'Diffuse 940nm for Security Cam', x, y); next_row(y);
      MenuParam(obj, 'Punishment', {'None','ExtraITI','60 psi',...
          '55 psi','50 psi','45 psi','40 psi','35 psi','30 psi','25 psi',...
          '20 psi','15 psi','10 psi','Quinine'},'ExtraITI', x, y); next_row(y);  
      MenuParam(obj, 'WhiskerSet', {'Full','C2 only','None','Row C only'},'Full', x, y); 
    
      
      % Record which rig experiment was run on:
      [status,host] = system('hostname');
      SoloParamHandle(obj, 'RigComputer', 'value', host);
      SoloParamHandle(obj, 'SaveTime');            
%       PushButtonParam(obj, 'loadsets', x, y, 'label', 'Load Settings');
%       set_callback(loadsets, {mfilename, 'loadsets'});
%       next_row(y);     
%       PushButtonParam(obj, 'savesets', x, y, 'label', 'Save Settings');
%       set_callback(savesets, {mfilename, 'savesets'});
%       next_row(y, 1.5);     
      

        MenuParam(obj, 'notes_show', {'view', 'hide'}, 'hide', x, y, 'label', 'Notes', 'TooltipString', 'Experiment notes');
        set_callback(notes_show, {mfilename,'hide_show'});


        SoloParamHandle(obj, 'notesfig', 'saveable', 0);
        notesfig.value = figure('Position', [10 500 200 400], 'Menubar', 'none',...
            'Toolbar', 'none','Name','Notes','NumberTitle','off');
        
        SoloParamHandle(obj, 'noteshandle');
        noteshandle.value = annotation(value(notesfig),'textbox',[.1 .1 .8 .8]);
        plotedit(value(notesfig));
        
        SoloParamHandle(obj, 'notes');
                
        SoloFunctionAddVars('SavingSection', 'rw_args', {'notes', 'noteshandle'});
        
        parentfig_x = x; parentfig_y = y;
        
        x = 1; y = 1;
        
        NotesSection(obj,'hide_show');
        
        x = parentfig_x; y = parentfig_y;
        set(0,'CurrentFigure',value(myfig));
        
            next_row(y);
        SubheaderParam(obj, 'title', 'Notes & Documentation', x, y);
        
        return;


    case 'hide_show'
        if strcmpi(value(notes_show), 'hide')
            set(value(notesfig), 'Visible', 'off');
        elseif strcmpi(value(notes_show),'view')
            set(value(notesfig),'Visible','on');
        end;
        return;


    case 'reinit',   % ------- CASE REINIT -------------
        currfig = gcf;

        % Get the original GUI position and figure:
        x = my_gui_info(1); y = my_gui_info(2); figure(my_gui_info(3));

        delete(value(myaxes));

        % Delete all SoloParamHandles who belong to this object and whose
        % fullname starts with the name of this mfile:
        delete_sphandle('owner', ['^@' class(obj) '$'], ...
            'fullname', ['^' mfilename]);

        % Reinitialise at the original GUI position and figure:
        [x, y] = feval(mfilename, obj, 'init', x, y);

        % Restore the current figure:
        figure(currfig);
        return;
end


