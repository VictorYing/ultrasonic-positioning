s = serial('COM14');
set(s,'BaudRate',9600);
set(s,'Timeout',1000);
set(s,'Terminator', 'CR/LF')
fopen(s);

while s.BytesAvailable < 16
end
A = fscanf(s,'X:%e Y:%e ');
x(1) = A(1);
y(1) = A(2);

plot(x, y, '-o');
linkdata on;

for i=2:1000
    if (s.BytesAvailable > 15)
        A = fscanf(s,'\nX:%e Y:%e ');
        x(i) = A(1);
        y(i) = A(2);
    end
end

linkdata off

fclose(s);
delete(s)
clear s