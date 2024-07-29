function loginFunc (document) {
    var user = document.getElementById("username");
    var pass = document.getElementById("password");
    $.ajax({
        type: 'post',
        url: '/',
        contentType: "application/json",
        data: JSON.stringify({
            type: 'login',
            username: user.value,
            password: pass.value
        }),
        success: function (data) {
            data = data.slice(0, -1);
            console.log(data);
            if (data === "login:error_name") {
                window.alert("用户名错误");
            }
            else if (data === "login:error_passd") {
                window.alert("密码错误");
            }
            else if (data === "login:success") {
                // 跳转
                location.href="/main.html";
            }
            else if (data.slice(0, -4) === "login:server_error") {
                window.alert(data);
            }
            else {
                window.alert("Bad Recieve.");
            }
        }
    });
}

function addFunc (document) {
    var user = document.getElementById("username");
    var pass = document.getElementById("password");
    $.ajax({
        type: 'post',
        url: '/',
        contentType: "application/json",
        data: JSON.stringify({
            type: 'add',
            username: user.value,
            password: pass.value
        }),
        success: function (data) {
            data = data.slice(0, -1);
            console.log(data);
            if (data == "add:exist_name") {
                window.alert("用户名已存在");
            }
            else if (data == "add:success") {
                window.alert("注册成功");
            }
            else if (data.slice(0, -4) === "add:server_error") {
                window.alert(data);
            }
            else {
                window.alert("Bad Recieve.");
            }
        }
    });
}
