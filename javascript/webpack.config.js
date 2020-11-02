module.exports = {
  entry: {
    live_demo: './src/live_demo.js',
    evaluation: './src/evaluation.js',
  },
  module: {
    rules: [
      {
        test: /\.(js)$/,
        exclude: /node_modules/,
        use: ['babel-loader']
      },
      {
        test: /\.js$/,
        loader: 'ify-loader'
      }
    ]
  },
  resolve: {
    extensions: ['*', '.js']
  },
  output: {
    path: __dirname + '/dist',
    publicPath: '/',
    filename: '[name].js',
    library: 'magnetics',
    libraryTarget:'umd'
  },
  devServer: {
    contentBase: './dist'
  }
};
