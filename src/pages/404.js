import React from 'react';
import { Link } from 'gatsby';
import Layout from '../components/Layout';

const NotFoundPage = () => (
  <Layout>
    <div>
      <h1>Page Not Found</h1>
      <p>
        The page you were looking for doesn't exist.{' '}
        <Link to="/">Return home</Link> or{' '}
        <Link to="/blog">browse the blog</Link>.
      </p>
    </div>
  </Layout>
);

export const Head = () => <title>404 Not Found | Interrupt</title>;

export default NotFoundPage;
