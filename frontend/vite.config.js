import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import { exec } from 'child_process';
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

function compilerApiPlugin() {
  return {
    name: 'compiler-api-plugin',
    configureServer(server) {
      server.middlewares.use('/api/compile', (req, res, next) => {
        if (req.method === 'POST') {
          let body = '';
          req.on('data', chunk => {
            body += chunk;
          });
          req.on('end', () => {
            try {
              const parsed = JSON.parse(body || '{}');
              const code = parsed.code || '';
              const projectRoot = path.resolve(__dirname, '..');
              const tempFilePath = path.join(projectRoot, 'temp_web.toy');
              fs.writeFileSync(tempFilePath, code, 'utf8');

              const compilerExe = path.join(projectRoot, 'bin', 'compiler.exe');

              exec(`"${compilerExe}" "${tempFilePath}"`, { cwd: projectRoot, timeout: 8000 }, (error, stdout, stderr) => {
                let astBase64 = null;
                const astPngPath = path.join(projectRoot, 'ast.png');
                if (fs.existsSync(astPngPath)) {
                  try {
                    const data = fs.readFileSync(astPngPath);
                    astBase64 = `data:image/png;base64,${data.toString('base64')}`;
                  } catch (e) {
                    console.error('Failed to read ast.png:', e);
                  }
                }

                try {
                  if (fs.existsSync(tempFilePath)) {
                    fs.unlinkSync(tempFilePath);
                  }
                } catch (e) {}

                res.setHeader('Content-Type', 'application/json');
                res.end(JSON.stringify({
                  success: !error,
                  exitCode: error ? error.code : 0,
                  stdout: stdout || '',
                  stderr: stderr || '',
                  astImage: astBase64,
                }));
              });
            } catch (err) {
              res.statusCode = 500;
              res.setHeader('Content-Type', 'application/json');
              res.end(JSON.stringify({ success: false, error: err.message }));
            }
          });
        } else {
          next();
        }
      });
    }
  };
}

export default defineConfig({
  plugins: [react(), compilerApiPlugin()],
  server: {
    port: 5173,
    host: true,
  }
});
