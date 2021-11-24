function[a,b] = f(x,y)
Sx = 0;
Sy = 0;
Sxy = 0;
Sx2 = 0;
n = length(x);
for i=1 : n
    Sx = Sx + x(i);
    Sx2 = Sx2 + x(i)*x(i);
end
for j=1 : n
    Sy = Sy + y(j);
end
for k=1 : n
    Sxy = Sxy + x(k)*y(k);
end
a = (n*Sxy-Sx*Sy)/(n*Sx2-Sx^2);
b = (Sx2*Sy-Sx*Sxy)/(n*Sx2-Sx^2);
end
%ovo je kod za linearizaciju karakteristike 
%pomo?u metode najmanjih kvadrata