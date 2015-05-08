if (~isempty(instrfind))
    fclose(instrfind);
    delete(instrfind);
    clear s;
end

s = serial('COM14');
s.BaudRate = 9600;
s.Timeout = 10;
s.Terminator = 'LF';
fopen(s);

h = animatedline('Color','red','Marker','o');

while 1
    try
        [str, count, msg] = fgetl(s);
        if(~isempty(msg))
            error(msg);
        end
    catch err
        err
        disp('A timeout occurred or the user quit the program!')
        break;
    end
    
    if str < 0
        break;
    end
    
    if str(1) == char(1)
        str = str(2:end);
    end
    [c, num] = sscanf(str, '%c%f%c%f', 4);
    if num > 0 && c(1) == 'X'
         x = c(2);
         y = c(4);
         addpoints(h, x, y);
         drawnow
    end
end

fclose(s);
delete(s)
clear s
clear h