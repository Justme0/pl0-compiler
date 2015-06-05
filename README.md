<html>
<body>
<div class="md-section-divider"></div>

<div class="md-section-divider"></div>

<h1 data-anchor-id="4h1m" id="src-文件夹内容">src 文件夹内容</h1>

<table data-anchor-id="ta79" class="table table-striped-white table-bordered">
<thead>
<tr>
 <th>程序名</th>
 <th>说明</th>
</tr>
</thead>
<tbody><tr>
 <td>compiler.c</td>
 <td>将源程序编译成中间代码，输出到文件，扩展名是 pl0c</td>
</tr>
<tr>
 <td>interpreter.c</td>
 <td>解释执行中间代码，结果输出到屏幕</td>
</tr>
<tr>
 <td>pl0.c</td>
 <td>合并两者，输入源程序，输出结果到屏幕</td>
</tr>
</tbody></table>


<div class="md-section-divider"></div>

<h1 data-anchor-id="hijx" id="test-文件夹内容">test 文件夹内容</h1>

<p data-anchor-id="huh6">若干 pl0 程序，作为测试程序。</p>

<div class="md-section-divider"></div>

<h1 data-anchor-id="7cf5" id="实现">实现</h1>

<ul data-anchor-id="uma4">
<li>数据栈的每个单元是32位</li>
<li><p>数据栈中各种数据的表示</p>

<table class="table table-striped-white table-bordered">
<thead>
<tr>
 <th>类型</th>
 <th>表示</th>
</tr>
</thead>
<tbody><tr>
 <td>integer</td>
 <td>用32位有符号数补码表示</td>
</tr>
<tr>
 <td>real</td>
 <td>用 IEEE 754 标准，32位表示</td>
</tr>
<tr>
 <td>Boolean</td>
 <td>为32位，true的内存表示为0x00000001，false为0x00000000</td>
</tr>
<tr>
 <td>地址（下标）</td>
 <td>用32位有符号数补码表示（应是正数）</td>
</tr>
</tbody></table>
</li>
<li>约定 <br>
<ul><li>odd, mod 只对 integer 操作</li>
<li>对 Boolean 的关系操作只有 eql 与 neq，不能比较大小</li></ul></li>
</ul>

<div class="md-section-divider"></div>

<h1 data-anchor-id="oaac" id="编译错误编号及含义">编译错误编号及含义</h1>

<table data-anchor-id="r74h" class="table table-striped-white table-bordered">
<thead>
<tr>
 <th>错误编号</th>
 <th>含义</th>
</tr>
</thead>
<tbody><tr>
 <td>1</td>
 <td>应为=而不是:=</td>
</tr>
<tr>
 <td>2</td>
 <td>=后应为数</td>
</tr>
<tr>
 <td>3</td>
 <td>标识符后应为=</td>
</tr>
<tr>
 <td>4</td>
 <td>const, type, var, procedure, function 后应为标识符</td>
</tr>
<tr>
 <td>5</td>
 <td>遗漏逗号或分号</td>
</tr>
<tr>
 <td>6</td>
 <td>过程声明后的记号不正确</td>
</tr>
<tr>
 <td>7</td>
 <td>应为语句</td>
</tr>
<tr>
 <td>8</td>
 <td>分程序内的语句部分后的记号不正确</td>
</tr>
<tr>
 <td>9</td>
 <td>应为句号</td>
</tr>
<tr>
 <td>10</td>
 <td>语句之间漏分号</td>
</tr>
<tr>
 <td>11</td>
 <td>标识符未声明</td>
</tr>
<tr>
 <td><del>12</del></td>
 <td><del>不可向常量或过程赋值</del></td>
</tr>
<tr>
 <td>13</td>
 <td>应为赋值运算符:=</td>
</tr>
<tr>
 <td>14</td>
 <td>call后应为标识符</td>
</tr>
<tr>
 <td>15</td>
 <td>call 后只能是过程名</td>
</tr>
<tr>
 <td>16</td>
 <td>应为then</td>
</tr>
<tr>
 <td>17</td>
 <td>应为分号或end</td>
</tr>
<tr>
 <td>18</td>
 <td>应为do</td>
</tr>
<tr>
 <td>19</td>
 <td>语句后的记号不正确</td>
</tr>
<tr>
 <td><del>20</del></td>
 <td><del>应为关系运算符</del></td>
</tr>
<tr>
 <td>21</td>
 <td>表达式内不可有过程标识符</td>
</tr>
<tr>
 <td>22</td>
 <td>遗漏括号</td>
</tr>
<tr>
 <td>23</td>
 <td>因子后不可为此记号</td>
</tr>
<tr>
 <td>24</td>
 <td>表达式不能以此记号开始</td>
</tr>
<tr>
 <td>30</td>
 <td>这个数太大</td>
</tr>
<tr>
 <td>31</td>
 <td>这个常数或地址偏移太大</td>
</tr>
<tr>
 <td>32</td>
 <td>程序嵌套层次太多</td>
</tr>
<tr>
 <td>33</td>
 <td>格式错误，应是右括号</td>
</tr>
<tr>
 <td>34</td>
 <td>格式错误，应是左括号</td>
</tr>
<tr>
 <td>35</td>
 <td>read()中应是声明过的变量名</td>
</tr>
<tr>
 <td>36</td>
 <td>实数的小数点后必须含非空数字串</td>
</tr>
<tr>
 <td>37</td>
 <td>定义变量或函数时缺少冒号</td>
</tr>
<tr>
 <td>38</td>
 <td>非法类型</td>
</tr>
<tr>
 <td>39</td>
 <td>类型不匹配</td>
</tr>
<tr>
 <td>40</td>
 <td>重定义</td>
</tr>
<tr>
 <td>41</td>
 <td>对该类型的操作数的操作未定义</td>
</tr>
<tr>
 <td>42</td>
 <td>整数除法应该用 div</td>
</tr>
<tr>
 <td>43</td>
 <td>odd 的操作数类型应是 integer</td>
</tr>
<tr>
 <td>44</td>
 <td>not 的操作数类型应是 Boolean</td>
</tr>
<tr>
 <td>45</td>
 <td>if/while的测试条件的类型应是 Boolean（在表达式计算后才能指出，指示位置有偏移）</td>
</tr>
<tr>
 <td>46</td>
 <td>只能输入输出整型或实型数据</td>
</tr>
<tr>
 <td>47</td>
 <td>应是关键字 array，只允许给数组类型命名</td>
</tr>
<tr>
 <td>48</td>
 <td>格式错误，应是左中括号</td>
</tr>
<tr>
 <td>49</td>
 <td>应是无符号整数,作为数组的下标界限</td>
</tr>
<tr>
 <td>50</td>
 <td>应是..</td>
</tr>
<tr>
 <td>51</td>
 <td>格式错误，应是右中括号</td>
</tr>
<tr>
 <td>52</td>
 <td>应是 of</td>
</tr>
<tr>
 <td>53</td>
 <td>数组下标应是整数</td>
</tr>
<tr>
 <td>54</td>
 <td>标识符太长（最多为al=10个字符）</td>
</tr>
<tr>
 <td>55</td>
 <td>应是类型标识符</td>
</tr>
<tr>
 <td>56</td>
 <td>非数组类型</td>
</tr>
<tr>
 <td>57</td>
 <td>标识符引用有误</td>
</tr>
<tr>
 <td>58</td>
 <td>只能修改变量或返回函数值</td>
</tr>
<tr>
 <td>59</td>
 <td>应是标识符</td>
</tr>
<tr>
 <td>60</td>
 <td>实参数目太多</td>
</tr>
<tr>
 <td>61</td>
 <td>实参数目太少</td>
</tr>
<tr>
 <td>62</td>
 <td>实参与形参类型不匹配</td>
</tr>
<tr>
 <td>63</td>
 <td>break 语句没有处于任何 while 语句中</td>
</tr>
<tr>
 <td>64</td>
 <td>函数的参数类型只能是 integer, real 或 Boolean</td>
</tr>
<tr>
 <td>65</td>
 <td>函数的返回值类型能是 integer, real 或 Boolean</td>
</tr>
</tbody></table>


<div class="md-section-divider"></div>

<h1 data-anchor-id="v6s8" id="新增指令含义">新增指令含义</h1>

<table data-anchor-id="0kkp" class="table table-striped-white table-bordered">
<thead>
<tr>
 <th>指令</th>
 <th>含义</th>
</tr>
</thead>
<tbody><tr>
 <td>jp0</td>
 <td>栈顶为0时转，此时 Boolean 表达式的值就是栈顶值，跳转后还需要</td>
</tr>
<tr>
 <td>jp1</td>
 <td>栈顶为1时转，此时 Boolean 表达式的值就是栈顶值，跳转后还需要</td>
</tr>
<tr>
 <td>lda</td>
 <td>load for a[i]</td>
</tr>
<tr>
 <td>sta</td>
 <td>store for a[i]</td>
</tr>
<tr>
 <td>ckb</td>
 <td>check bounds, 单独作为一条指令可能不合适</td>
</tr>
<tr>
 <td>ppa</td>
 <td>pop arguments, 函数调用完成后栈顶指针恢复到参数压栈前的位置，address 域为参数个数</td>
</tr>
<tr>
 <td>rva</td>
 <td>reverse arguments（真正实现时可能不用这样），address 域为参数个数</td>
</tr>
</tbody></table>


<div class="md-section-divider"></div>

<h1 data-anchor-id="8rs7" id="opr-指令的-address-域含义">opr 指令的 address 域含义</h1>

<table data-anchor-id="wlwz" class="table table-striped-white table-bordered">
<thead>
<tr>
 <th>address</th>
 <th>含义</th>
</tr>
</thead>
<tbody><tr>
 <td>0</td>
 <td>返回</td>
</tr>
<tr>
 <td>1</td>
 <td>integer 取相反数</td>
</tr>
<tr>
 <td>2</td>
 <td>integer +（--t, s[t] = s[t] op s[t+1]，下面同理）</td>
</tr>
<tr>
 <td>3</td>
 <td>integer -</td>
</tr>
<tr>
 <td>4</td>
 <td>integer *</td>
</tr>
<tr>
 <td>5</td>
 <td>integer div</td>
</tr>
<tr>
 <td>6</td>
 <td>integer odd</td>
</tr>
<tr>
 <td>8</td>
 <td>integer ==<br>or real ==（对实数的比较，只有内存表示相同时才认为相等，TODO:此时应给个警告)<br>or Boolean ==</td>
</tr>
<tr>
 <td>9</td>
 <td>integer !=<br>or real !=<br>or Boolean !=</td>
</tr>
<tr>
 <td>10</td>
 <td>integer &lt;</td>
</tr>
<tr>
 <td>11</td>
 <td>integer &gt;=</td>
</tr>
<tr>
 <td>12</td>
 <td>integer &gt;</td>
</tr>
<tr>
 <td>13</td>
 <td>integer &lt;=</td>
</tr>
<tr>
 <td>14</td>
 <td>输出 integer</td>
</tr>
<tr>
 <td>15</td>
 <td>输出换行</td>
</tr>
<tr>
 <td>16</td>
 <td>输入 integer</td>
</tr>
<tr>
 <td>17</td>
 <td>输出 real</td>
</tr>
<tr>
 <td>18</td>
 <td>real 取相反数</td>
</tr>
<tr>
 <td>19</td>
 <td>real +</td>
</tr>
<tr>
 <td>20</td>
 <td>real -</td>
</tr>
<tr>
 <td>21</td>
 <td>real *</td>
</tr>
<tr>
 <td>22</td>
 <td>real /</td>
</tr>
<tr>
 <td>23</td>
 <td>输入 real</td>
</tr>
<tr>
 <td>24</td>
 <td>读掉剩余字符（忽略当前输入行剩余字符，下一个读语句接受的数据另起一行）</td>
</tr>
<tr>
 <td>26</td>
 <td>real &lt;</td>
</tr>
<tr>
 <td>27</td>
 <td>real &gt;=</td>
</tr>
<tr>
 <td>28</td>
 <td>real &gt;</td>
</tr>
<tr>
 <td>29</td>
 <td>real &lt;=</td>
</tr>
<tr>
 <td>30</td>
 <td>convert integer(s[t]) to real</td>
</tr>
<tr>
 <td>31</td>
 <td>convert integer(s[t-1]) to real</td>
</tr>
<tr>
 <td>32</td>
 <td>Boolean or</td>
</tr>
<tr>
 <td>33</td>
 <td>Boolean and</td>
</tr>
<tr>
 <td>34</td>
 <td>Boolean not</td>
</tr>
<tr>
 <td>35</td>
 <td>integer mod</td>
</tr>
<tr>
 <td>36</td>
 <td>将栈顶值存到寄存器</td>
</tr>
<tr>
 <td>37</td>
 <td>从寄存器取值到栈顶</td>
</tr>
</tbody></table>
</body>
</html>
