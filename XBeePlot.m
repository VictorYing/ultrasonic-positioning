if (~isempty(instrfind))
    fclose(instrfind);
    delete(instrfind);
    clear s;
end

s = serial('COM14');
s.BaudRate = 9600;
s.Timeout = 20;
s.Terminator = 'LF';
fopen(s);

figure(1);
xlim([-12 12]);
ylim([-17 17]);
pbaspect([1 1 1]);
clear h;
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
    
    for i=1:size(str)
        if str(i) ~= 'X'
        else 
            str = str(i:end);
            break;
        end
    end
    if strcmp(str, '')
        continue;
    end
    [c, num] = sscanf(str, '%c%f%c%f', 4);
    if num == 4
         x = c(2);
         y = c(4);
         if (x ~= 0 || y ~= 0)
            addpoints(h, x, y);
            drawnow
         end
    end
end

fclose(s);
delete(s)
clear s