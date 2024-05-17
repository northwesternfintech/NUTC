import { authRequired } from '../utils/withAuth'

async function test() {
  return <h1>Test</h1>
}

export default authRequired(test)
