from flask import Flask
app = Flask(__name__)

@app.route('/')
def hello_world():
    return '''
    <!doctype html>
<title>RAI</title>
<link rel="stylesheet" href="{{ url_for('static', filename='style.css') }}">
<nav>
  <h1>Respirador Artificial Integrado</h1>
  <ul>
    <li><a href="{{ url_for('auth.login') }}">Log In</a>
    <li><a href="{{ url_for('auth.register') }}">Register</a>
  </ul>
</nav>
<section class="content">
  <header>
    Descrição
  </header>
    Dados
</section>
'''