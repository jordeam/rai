import os

from flask import Flask


# creates the Flask instance
def create_app(test_config=None):
    # create and configure the app
    app = Flask(__name__, instance_relative_config=True)
    app.config.from_mapping(
        SECRET_KEY='dev',
        DATABASE=os.path.join(app.instance_path, 'flaskr.sqlite'),
    )

    if test_config is None:
        # load the instance config, if it exists, when not testing
        app.config.from_pyfile('config.py', silent=True)
    else:
        # load the test config if passed in
        app.config.from_mapping(test_config)

    # ensure the instance folder exists
    try:
        os.makedirs(app.instance_path)
    except OSError:
        pass

    # a simple page that says hello
    @app.route('/hello')
    def hello():
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

    return app
